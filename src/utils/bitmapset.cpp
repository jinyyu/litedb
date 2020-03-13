#include <litedb/utils/bitmapset.h>
#include <litedb/utils/elog.h>
#include <litedb/utils/env.h>
#include <litedb/int.h>
#include <litedb/utils/list.h>

namespace db {
typedef u64 bitmapword;        /* must be an unsigned type */
typedef i64 signedbitmapword;  /* must be the matching signed type */

struct Bitmapset {
  int nwords;           /* number of words in array */
  bitmapword words[0];  /* really [nwords] */
};

#define BITS_PER_BITMAPWORD 64

#define WORDNUM(x)    ((x) / BITS_PER_BITMAPWORD)
#define BITNUM(x)    ((x) % BITS_PER_BITMAPWORD)

#define BITMAPSET_SIZE(nwords)    (sizeof(Bitmapset) + (nwords) * sizeof(bitmapword))

/*----------
 * This is a well-known cute trick for isolating the rightmost one-bit
 * in a word.  It assumes two's complement arithmetic.  Consider any
 * nonzero value, and focus attention on the rightmost one.  The value is
 * then something like
 *				xxxxxx10000
 * where x's are unspecified bits.  The two's complement negative is formed
 * by inverting all the bits and adding one.  Inversion gives
 *				yyyyyy01111
 * where each y is the inverse of the corresponding x.  Incrementing gives
 *				yyyyyy10000
 * and then ANDing with the original value gives
 *				00000010000
 * This works for all cases except original value = zero, where of course
 * we get zero.
 *----------
 */
#define RIGHTMOST_ONE(x) ((signedbitmapword) (x) & -((signedbitmapword) (x)))

#define HAS_MULTIPLE_ONES(x)    ((bitmapword) RIGHTMOST_ONE(x) != (x))

/*
 * bms_copy - make a palloc'd copy of a bitmapset
 */
Bitmapset* bms_copy(const Bitmapset* a) {
  Bitmapset* result;
  size_t size;

  if (a == NULL) {
    return NULL;
  }

  size = BITMAPSET_SIZE(a->nwords);
  result = (Bitmapset*) SessionEnv->Malloc0(size);
  memcpy(result, a, size);
  return result;
}

/*
 * bms_make_singleton - build a bitmapset containing a single member
 */
Bitmapset* bms_make_singleton(int x) {
  Bitmapset* result;
  int wordnum;
  int bitnum;

  if (x < 0) {
    elog(ERROR, "negative bitmapset member not allowed");
  }

  wordnum = WORDNUM(x);
  bitnum = BITNUM(x);
  result = (Bitmapset*) SessionEnv->Malloc0(BITMAPSET_SIZE(wordnum + 1));
  result->nwords = wordnum + 1;
  result->words[wordnum] = ((bitmapword) 1 << bitnum);
  return result;
}

/*
 * bms_free - free a bitmapset
 *
 * Same as free except for allowing NULL input
 */
void bms_free(Bitmapset* a) {
  if (a) {
    SessionEnv->Free(a);
  }
}

/*
 * bms_equal - are two bitmapsets equal?
 *
 * This is logical not physical equality; in particular, a NULL pointer will
 * be reported as equal to a palloc'd value containing no members.
 */
bool bms_equal(const Bitmapset* a, const Bitmapset* b) {
  const Bitmapset* shorter;
  const Bitmapset* longer;
  int shortlen;
  int longlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    if (b == NULL) {
      return true;
    }
    return bms_is_empty(b);
  } else if (b == NULL) {
    return bms_is_empty(a);
  }

  /* Identify shorter and longer input */
  if (a->nwords <= b->nwords) {
    shorter = a;
    longer = b;
  } else {
    shorter = b;
    longer = a;
  }
  /* And process */
  shortlen = shorter->nwords;
  for (i = 0; i < shortlen; i++) {
    if (shorter->words[i] != longer->words[i]) {
      return false;
    }
  }
  longlen = longer->nwords;
  for (; i < longlen; i++) {
    if (longer->words[i] != 0) {
      return false;
    }
  }
  return true;
}

/*
 * bms_compare - qsort-style comparator for bitmapsets
 *
 * This guarantees to report values as equal iff bms_equal would say they are
 * equal.  Otherwise, the highest-numbered bit that is set in one value but
 * not the other determines the result.  (This rule means that, for example,
 * {6} is greater than {5}, which seems plausible.)
 */
int bms_compare(const Bitmapset* a, const Bitmapset* b) {
  int shortlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return bms_is_empty(b) ? 0 : -1;
  } else if (b == NULL) {
    return bms_is_empty(a) ? 0 : +1;
  }

  /* Handle cases where one input is longer than the other */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = shortlen; i < a->nwords; i++) {
    if (a->words[i] != 0) {
      return +1;
    }
  }
  for (i = shortlen; i < b->nwords; i++) {
    if (b->words[i] != 0) {
      return -1;
    }
  }
  /* Process words in common */
  i = shortlen;
  while (--i >= 0) {
    bitmapword aw = a->words[i];
    bitmapword bw = b->words[i];

    if (aw != bw) {
      return (aw > bw) ? +1 : -1;
    }
  }
  return 0;
}

/*
 * bms_membership - does a set have zero, one, or multiple members?
 *
 * This is faster than making an exact count with bms_num_members().
 */
BMS_Membership bms_membership(const Bitmapset* a) {
  BMS_Membership result = BMS_EMPTY_SET;
  int nwords;
  int wordnum;

  if (a == NULL) {
    return BMS_EMPTY_SET;
  }

  nwords = a->nwords;
  for (wordnum = 0; wordnum < nwords; wordnum++) {
    bitmapword w = a->words[wordnum];

    if (w != 0) {
      if (result != BMS_EMPTY_SET || HAS_MULTIPLE_ONES(w)) {
        return BMS_MULTIPLE;
      }
      result = BMS_SINGLETON;
    }
  }
  return result;
}

/*
 * bms_is_empty - is a set empty?
 *
 * This is even faster than bms_membership().
 */
bool bms_is_empty(const Bitmapset* a) {
  int nwords;
  int wordnum;

  if (a == NULL) {
    return true;
  }
  nwords = a->nwords;
  for (wordnum = 0; wordnum < nwords; wordnum++) {
    bitmapword w = a->words[wordnum];

    if (w != 0) {
      return false;
    }
  }
  return true;
}

/*
 * These operations all make a freshly palloc'd result,
 * leaving their inputs untouched
 */


/*
 * bms_union - set union
 */
Bitmapset* bms_union(const Bitmapset* a, const Bitmapset* b) {
  Bitmapset* result;
  const Bitmapset* other;
  int otherlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return bms_copy(b);
  }
  if (b == NULL) {
    return bms_copy(a);
  }
  /* Identify shorter and longer input; copy the longer one */
  if (a->nwords <= b->nwords) {
    result = bms_copy(b);
    other = a;
  } else {
    result = bms_copy(a);
    other = b;
  }
  /* And union the shorter input into the result */
  otherlen = other->nwords;
  for (i = 0; i < otherlen; i++) {
    result->words[i] |= other->words[i];
  }
  return result;
}

/*
 * bms_intersect - set intersection
 */
Bitmapset* bms_intersect(const Bitmapset* a, const Bitmapset* b) {
  Bitmapset* result;
  const Bitmapset* other;
  int resultlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL || b == NULL) {
    return NULL;
  }

  /* Identify shorter and longer input; copy the shorter one */
  if (a->nwords <= b->nwords) {
    result = bms_copy(a);
    other = b;
  } else {
    result = bms_copy(b);
    other = a;
  }
  /* And intersect the longer input with the result */
  resultlen = result->nwords;
  for (i = 0; i < resultlen; i++) {
    result->words[i] &= other->words[i];
  }
  return result;
}

/*
 * bms_difference - set difference (ie, A without members of B)
 */
Bitmapset* bms_difference(const Bitmapset* a, const Bitmapset* b) {
  Bitmapset* result;
  int shortlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return NULL;
  }

  if (b == NULL) {
    return bms_copy(a);
  }

  /* Copy the left input */
  result = bms_copy(a);
  /* And remove b's bits from result */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    result->words[i] &= ~b->words[i];
  }
  return result;
}

/*
 * bms_is_subset - is A a subset of B?
 */
bool bms_is_subset(const Bitmapset* a, const Bitmapset* b) {
  int shortlen;
  int longlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return true;            /* empty set is a subset of anything */
  }

  if (b == NULL) {
    return bms_is_empty(a);
  }

  /* Check common words */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    if ((a->words[i] & ~b->words[i]) != 0) {
      return false;
    }
  }
  /* Check extra words */
  if (a->nwords > b->nwords) {
    longlen = a->nwords;
    for (; i < longlen; i++) {
      if (a->words[i] != 0) {
        return false;
      }
    }
  }
  return true;
}

/*
 * bms_subset_compare - compare A and B for equality/subset relationships
 *
 * This is more efficient than testing bms_is_subset in both directions.
 */
BMS_Comparison bms_subset_compare(const Bitmapset* a, const Bitmapset* b) {
  BMS_Comparison result;
  int shortlen;
  int longlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    if (b == NULL)
      return BMS_EQUAL;
    return bms_is_empty(b) ? BMS_EQUAL : BMS_SUBSET1;
  }
  if (b == NULL)
    return bms_is_empty(a) ? BMS_EQUAL : BMS_SUBSET2;
  /* Check common words */
  result = BMS_EQUAL;            /* status so far */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    bitmapword aword = a->words[i];
    bitmapword bword = b->words[i];

    if ((aword & ~bword) != 0) {
      /* a is not a subset of b */
      if (result == BMS_SUBSET1)
        return BMS_DIFFERENT;
      result = BMS_SUBSET2;
    }
    if ((bword & ~aword) != 0) {
      /* b is not a subset of a */
      if (result == BMS_SUBSET2)
        return BMS_DIFFERENT;
      result = BMS_SUBSET1;
    }
  }
  /* Check extra words */
  if (a->nwords > b->nwords) {
    longlen = a->nwords;
    for (; i < longlen; i++) {
      if (a->words[i] != 0) {
        /* a is not a subset of b */
        if (result == BMS_SUBSET1)
          return BMS_DIFFERENT;
        result = BMS_SUBSET2;
      }
    }
  } else if (a->nwords < b->nwords) {
    longlen = b->nwords;
    for (; i < longlen; i++) {
      if (b->words[i] != 0) {
        /* b is not a subset of a */
        if (result == BMS_SUBSET2)
          return BMS_DIFFERENT;
        result = BMS_SUBSET1;
      }
    }
  }
  return result;
}

bool bms_is_member(int x, const Bitmapset* a) {
  int wordnum;
  int bitnum;

  /* XXX better to just return false for x<0 ? */
  if (x < 0) {
    elog(ERROR, "negative bitmapset member not allowed");
  }
  if (a == NULL) {
    return false;
  }
  wordnum = WORDNUM(x);
  bitnum = BITNUM(x);
  if (wordnum >= a->nwords) {
    return false;
  }
  if ((a->words[wordnum] & ((bitmapword) 1 << bitnum)) != 0) {
    return true;
  }
  return false;
}

/*
 * bms_overlap - do sets overlap (ie, have a nonempty intersection)?
 */
bool bms_overlap(const Bitmapset* a, const Bitmapset* b) {
  int shortlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL || b == NULL) {
    return false;
  }
  /* Check words in common */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    if ((a->words[i] & b->words[i]) != 0) {
      return true;
    }
  }
  return false;
}

/*
 * bms_overlap_list - does a set overlap an integer list?
 */
bool bms_overlap_list(const Bitmapset* a, const List* b) {
  ListCell* lc;
  int wordnum;
  int bitnum;

  if (a == NULL || b == NULL) {
    return false;
  }

  foreach(lc, b) {
    int x = lfirst_int(lc);

    if (x < 0) {
      elog(ERROR, "negative bitmapset member not allowed");
    }
    wordnum = WORDNUM(x);
    bitnum = BITNUM(x);
    if (wordnum < a->nwords) {
      if ((a->words[wordnum] & ((bitmapword) 1 << bitnum)) != 0) {
        return true;
      }
    }
  }

  return false;
}

/*
 * bms_nonempty_difference - do sets have a nonempty difference?
 */
bool bms_nonempty_difference(const Bitmapset* a, const Bitmapset* b) {
  int shortlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return false;
  }
  if (b == NULL) {
    return !bms_is_empty(a);
  }
  /* Check words in common */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    if ((a->words[i] & ~b->words[i]) != 0)
      return true;
  }
  /* Check extra words in a */
  for (; i < a->nwords; i++) {
    if (a->words[i] != 0) {
      return true;
    }
  }
  return false;
}

/*
 * bms_add_member - add a specified member to set
 *
 * Input set is modified or recycled!
 */
Bitmapset* bms_add_member(Bitmapset* a, int x) {
  int wordnum,
      bitnum;

  if (x < 0) {
    elog(ERROR, "negative bitmapset member not allowed");
  }
  if (a == NULL) {
    return bms_make_singleton(x);
  }
  wordnum = WORDNUM(x);
  bitnum = BITNUM(x);

  /* enlarge the set if necessary */
  if (wordnum >= a->nwords) {
    int oldnwords = a->nwords;
    int i;

    a = (Bitmapset*) SessionEnv->Realloc((void*) a, BITMAPSET_SIZE(wordnum + 1));
    a->nwords = wordnum + 1;
    /* zero out the enlarged portion */
    for (i = oldnwords; i < a->nwords; i++)
      a->words[i] = 0;
  }

  a->words[wordnum] |= ((bitmapword) 1 << bitnum);
  return a;
}

/*
 * bms_del_member - remove a specified member from set
 *
 * No error if x is not currently a member of set
 *
 * Input set is modified in-place!
 */
Bitmapset* bms_del_member(Bitmapset* a, int x) {
  int wordnum,
      bitnum;

  if (x < 0) {
    elog(ERROR, "negative bitmapset member not allowed");
  }
  if (a == NULL) {
    return NULL;
  }
  wordnum = WORDNUM(x);
  bitnum = BITNUM(x);
  if (wordnum < a->nwords) {
    a->words[wordnum] &= ~((bitmapword) 1 << bitnum);
  }
  return a;
}

/*
 * bms_add_members - like bms_union, but left input is recycled
 */
Bitmapset* bms_add_members(Bitmapset* a, const Bitmapset* b) {
  Bitmapset* result;
  const Bitmapset* other;
  int otherlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return bms_copy(b);
  }
  if (b == NULL) {
    return a;
  }
  /* Identify shorter and longer input; copy the longer one if needed */
  if (a->nwords < b->nwords) {
    result = bms_copy(b);
    other = a;
  } else {
    result = a;
    other = b;
  }
  /* And union the shorter input into the result */
  otherlen = other->nwords;
  for (i = 0; i < otherlen; i++) {
    result->words[i] |= other->words[i];
  }
  if (result != a) {
    SessionEnv->Free(a);
  }
  return result;
}

/*
 * bms_add_range
 *		Add members in the range of 'lower' to 'upper' to the set.
 *
 * Note this could also be done by calling bms_add_member in a loop, however,
 * using this function will be faster when the range is large as we work at
 * the bitmapword level rather than at bit level.
 */
Bitmapset*
bms_add_range(Bitmapset* a, int lower, int upper) {
  int lwordnum,
      lbitnum,
      uwordnum,
      ushiftbits,
      wordnum;

  /* do nothing if nothing is called for, without further checking */
  if (upper < lower)
    return a;

  if (lower < 0) {
    elog(ERROR, "negative bitmapset member not allowed");
  }
  uwordnum = WORDNUM(upper);

  if (a == NULL) {
    a = (Bitmapset*) SessionEnv->Malloc0(BITMAPSET_SIZE(uwordnum + 1));
    a->nwords = uwordnum + 1;
  } else if (uwordnum >= a->nwords) {
    int oldnwords = a->nwords;
    int i;

    /* ensure we have enough words to store the upper bit */
    a = (Bitmapset*) SessionEnv->Realloc(a, BITMAPSET_SIZE(uwordnum + 1));
    a->nwords = uwordnum + 1;
    /* zero out the enlarged portion */
    for (i = oldnwords; i < a->nwords; i++) {
      a->words[i] = 0;
    }
  }

  wordnum = lwordnum = WORDNUM(lower);

  lbitnum = BITNUM(lower);
  ushiftbits = BITS_PER_BITMAPWORD - (BITNUM(upper) + 1);

  /*
   * Special case when lwordnum is the same as uwordnum we must perform the
   * upper and lower masking on the word.
   */
  if (lwordnum == uwordnum) {
    a->words[lwordnum] |= ~(bitmapword) (((bitmapword) 1 << lbitnum) - 1)
        & (~(bitmapword) 0) >> ushiftbits;
  } else {
    /* turn on lbitnum and all bits left of it */
    a->words[wordnum++] |= ~(bitmapword) (((bitmapword) 1 << lbitnum) - 1);

    /* turn on all bits for any intermediate words */
    while (wordnum < uwordnum) {
      a->words[wordnum++] = ~(bitmapword) 0;
    }

    /* turn on upper's bit and all bits right of it. */
    a->words[uwordnum] |= (~(bitmapword) 0) >> ushiftbits;
  }

  return a;
}

/*
 * bms_int_members - like bms_intersect, but left input is recycled
 */
Bitmapset* bms_int_members(Bitmapset* a, const Bitmapset* b) {
  int shortlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return NULL;
  }

  if (b == NULL) {
    SessionEnv->Free(a);
    return NULL;
  }
  /* Intersect b into a; we need never copy */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    a->words[i] &= b->words[i];
  }

  for (; i < a->nwords; i++) {
    a->words[i] = 0;
  }
  return a;
}

/*
 * bms_del_members - like bms_difference, but left input is recycled
 */
Bitmapset* bms_del_members(Bitmapset* a, const Bitmapset* b) {
  int shortlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return NULL;
  }

  if (b == NULL) {
    return a;
  }

  /* Remove b's bits from a; we need never copy */
  shortlen = std::min(a->nwords, b->nwords);
  for (i = 0; i < shortlen; i++) {
    a->words[i] &= ~b->words[i];
  }
  return a;
}

/*
 * bms_join - like bms_union, but *both* inputs are recycled
 */
Bitmapset* bms_join(Bitmapset* a, Bitmapset* b) {
  Bitmapset* result;
  Bitmapset* other;
  int otherlen;
  int i;

  /* Handle cases where either input is NULL */
  if (a == NULL) {
    return b;
  }

  if (b == NULL) {
    return a;
  }

  /* Identify shorter and longer input; use longer one as result */
  if (a->nwords < b->nwords) {
    result = b;
    other = a;
  } else {
    result = a;
    other = b;
  }
  /* And union the shorter input into the result */
  otherlen = other->nwords;
  for (i = 0; i < otherlen; i++) {
    result->words[i] |= other->words[i];
  }

  if (other != result) {
    /* pure paranoia */
    SessionEnv->Free(other);
  }
  return result;
}

}
