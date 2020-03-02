#ifndef LITEDB_UTILS_LIST_H_
#define LITEDB_UTILS_LIST_H_
#include <litedb/nodes/nodes.h>
#include <litedb/utils/env.h>

namespace db {

struct ListCell;

struct List {
  NodeTag type;            /* T_List, T_IntList */
  int length;
  ListCell* head;
  ListCell* tail;
};

struct ListCell {
  union {
    void* ptr_value;
    int int_value;
  } data;
  ListCell* next;
};

/*
 * These routines are used frequently. However, we can't implement
 * them as macros, since we want to avoid double-evaluation of macro
 * arguments.
 */
static inline ListCell* list_head(const List* l) {
  return l ? l->head : NULL;
}

static inline ListCell*
list_tail(List* l) {
  return l ? l->tail : NULL;
}

static inline int
list_length(const List* l) {
  return l ? l->length : 0;
}

#define lnext(lc)                ((lc)->next)
#define lfirst(lc)                ((lc)->data.ptr_value)
#define lfirst_int(lc)            ((lc)->data.int_value)

#define linitial(l)                lfirst(list_head(l))
#define linitial_int(l)            lfirst_int(list_head(l))

#define lsecond(l)                lfirst(lnext(list_head(l)))
#define lsecond_int(l)            lfirst_int(lnext(list_head(l)))

#define lthird(l)                lfirst(lnext(lnext(list_head(l))))
#define lthird_int(l)            lfirst_int(lnext(lnext(list_head(l))))

#define lfourth(l)                lfirst(lnext(lnext(lnext(list_head(l)))))
#define lfourth_int(l)            lfirst_int(lnext(lnext(lnext(list_head(l)))))

#define llast(l)                lfirst(list_tail(l))
#define llast_int(l)            lfirst_int(list_tail(l))

/*
 * Convenience macros for building fixed-length lists
 */
#define list_make1(x1)                lcons(x1, NULL)
#define list_make2(x1, x2)            lcons(x1, list_make1(x2))
#define list_make3(x1, x2, x3)        lcons(x1, list_make2(x2, x3))
#define list_make4(x1, x2, x3, x4)        lcons(x1, list_make3(x2, x3, x4))
#define list_make5(x1, x2, x3, x4, x5)    lcons(x1, list_make4(x2, x3, x4, x5))

/*
 * foreach -
 *	  a convenience macro which loops through the list
 */
#define foreach(cell, l)    \
    for ((cell) = list_head(l); (cell) != NULL; (cell) = lnext(cell))

List* lappend(List* list, void* datum);
List* lappend_int(List* list, int datum);

List* lcons(void* datum, List* list);
List* lcons_int(int datum, List* list);

List* list_concat(List* list1, List* list2);

}
#endif //LITEDB_UTILS_LIST_H_
