#include <litedb/utils/list.h>
#include <assert.h>
#include <litedb/utils/elog.h>

namespace db {


/*
 * Routines to simplify writing assertions about the type of a list; a
 * NIL list is considered to be an empty list of any type.
 */
#define IsPointerList(l)        ((l) == NULL || l->type == T_List)
#define IsIntegerList(l)        ((l) == NULL || l->type == T_IntList)

/*
 * Return a freshly allocated List. Since empty non-NIL lists are
 * invalid, new_list() also allocates the head cell of the new list:
 * the caller should be sure to fill in that cell's data.
 */
static List* new_list(NodeTag type) {
  List* new_list;
  ListCell* new_head;

  new_head = (ListCell*) SessionEnv->Malloc(sizeof(*new_head));
  new_head->next = NULL;
  /* new_head->data is left undefined! */

  new_list = (List*) SessionEnv->Malloc(sizeof(*new_list));
  new_list->type = type;
  new_list->length = 1;
  new_list->head = new_head;
  new_list->tail = new_head;

  return new_list;
}

/*
 * Allocate a new cell and make it the head of the specified
 * list. Assumes the list it is passed is non-NIL.
 *
 * The data in the new head cell is undefined; the caller should be
 * sure to fill it in
 */
static void new_head_cell(List* list) {
  ListCell* new_head;

  new_head = (ListCell*) SessionEnv->Malloc(sizeof(*new_head));
  new_head->next = list->head;

  list->head = new_head;
  list->length++;
}

/*
 * Allocate a new cell and make it the tail of the specified
 * list. Assumes the list it is passed is non-NIL.
 *
 * The data in the new tail cell is undefined; the caller should be
 * sure to fill it in
 */
static void new_tail_cell(List* list) {
  ListCell* new_tail;

  new_tail = (ListCell*) SessionEnv->Malloc(sizeof(*new_tail));
  new_tail->next = NULL;

  list->tail->next = new_tail;
  list->tail = new_tail;
  list->length++;
}

/*
 * Append a pointer to the list. A pointer to the modified list is
 * returned. Note that this function may or may not destructively
 * modify the list; callers should always use this function's return
 * value, rather than continuing to use the pointer passed as the
 * first argument.
 */
List* lappend(List* list, void* datum) {
  assert(IsPointerList(list));

  if (list == NULL)
    list = new_list(T_List);
  else
    new_tail_cell(list);

  lfirst(list->tail) = datum;
  return list;
}

/*
 * Append an integer to the specified list. See lappend()
 */
List* lappend_int(List* list, int datum) {
  assert(IsIntegerList(list));

  if (list == NULL)
    list = new_list(T_IntList);
  else
    new_tail_cell(list);

  lfirst_int(list->tail) = datum;
  return list;
}

/*
 * Prepend a new element to the list. A pointer to the modified list
 * is returned. Note that this function may or may not destructively
 * modify the list; callers should always use this function's return
 * value, rather than continuing to use the pointer passed as the
 * second argument.
 *
 * Caution: before Postgres 8.0, the original List was unmodified and
 * could be considered to retain its separate identity.  This is no longer
 * the case.
 */
List* lcons(void* datum, List* list) {
  assert(IsPointerList(list));

  if (list == NULL)
    list = new_list(T_List);
  else
    new_head_cell(list);

  lfirst(list->head) = datum;
  return list;
}

/*
 * Prepend an integer to the list. See lcons()
 */
List* lcons_int(int datum, List* list) {
  assert(IsIntegerList(list));

  if (list == NULL)
    list = new_list(T_IntList);
  else
    new_head_cell(list);

  lfirst_int(list->head) = datum;
  return list;
}

List* list_concat(List* list1, List* list2) {
  if (list1 == NULL)
    return list2;
  if (list2 == NULL)
    return list1;
  if (list1 == list2)
    elog(ERROR, "cannot list_concat() a list to itself");

  assert(list1->type == list2->type);

  list1->length += list2->length;
  list1->tail->next = list2->head;
  list1->tail = list2->tail;

  return list1;
}

}
