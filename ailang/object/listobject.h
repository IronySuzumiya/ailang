#pragma once
#ifndef LIST_OBJECT
#define LIST_OBJECT

#include "../system/utils.h"

typedef struct _listobject {
    OBJECT_VAR_HEAD
    Object **ob_item;
    ssize_t allocated;
}
ListObject;

#define CHECK_TYPE_LIST(ob) CHECK_TYPE(ob, &type_listobject)

#define NUMBER_FREE_LISTS_MAX 80

API_DATA(TypeObject) type_listobject;
API_DATA(ListObject *) free_lists[NUMBER_FREE_LISTS_MAX];
API_DATA(int) number_free_lists;
API_FUNC(Object *) list_new(ssize_t size);

#endif
