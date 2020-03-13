#ifndef LITEDB_UTILS_BITMAPSET_H_
#define LITEDB_UTILS_BITMAPSET_H_

namespace db {

struct Bitmapset;
struct List;

/* result of bms_subset_compare */
enum BMS_Comparison {
  BMS_EQUAL,                  /* sets are equal */
  BMS_SUBSET1,                /* first set is a subset of the second */
  BMS_SUBSET2,                /* second set is a subset of the first */
  BMS_DIFFERENT               /* neither set is a subset of the other */
};

/* result of bms_membership */
enum BMS_Membership {
  BMS_EMPTY_SET,                /* 0 members */
  BMS_SINGLETON,                /* 1 member */
  BMS_MULTIPLE                  /* >1 member */
};

Bitmapset* bms_copy(const Bitmapset* a);
bool bms_equal(const Bitmapset* a, const Bitmapset* b);
int bms_compare(const Bitmapset* a, const Bitmapset* b);
Bitmapset* bms_make_singleton(int x);
void bms_free(Bitmapset* a);

Bitmapset* bms_union(const Bitmapset* a, const Bitmapset* b);
Bitmapset* bms_intersect(const Bitmapset* a, const Bitmapset* b);
Bitmapset* bms_difference(const Bitmapset* a, const Bitmapset* b);
bool bms_is_subset(const Bitmapset* a, const Bitmapset* b);
BMS_Comparison bms_subset_compare(const Bitmapset* a, const Bitmapset* b);
bool bms_is_member(int x, const Bitmapset* a);
bool bms_overlap(const Bitmapset* a, const Bitmapset* b);
bool bms_overlap_list(const Bitmapset* a, const struct List* b);
bool bms_nonempty_difference(const Bitmapset* a, const Bitmapset* b);

/* optimized tests when we don't need to know exact membership count: */
BMS_Membership bms_membership(const Bitmapset* a);
bool bms_is_empty(const Bitmapset* a);

/* these routines recycle (modify or free) their non-const inputs: */

Bitmapset* bms_add_member(Bitmapset* a, int x);
Bitmapset* bms_del_member(Bitmapset* a, int x);
Bitmapset* bms_add_members(Bitmapset* a, const Bitmapset* b);
Bitmapset* bms_add_range(Bitmapset* a, int lower, int upper);
Bitmapset* bms_int_members(Bitmapset* a, const Bitmapset* b);
Bitmapset* bms_del_members(Bitmapset* a, const Bitmapset* b);
Bitmapset* bms_join(Bitmapset* a, Bitmapset* b);

}

#endif //LITEDB_UTILS_BITMAPSET_H_
