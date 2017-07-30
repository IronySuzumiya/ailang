#pragma once
#ifndef LIST_OBJECT
#define LIST_OBJECT

#include "../system/utils.h"

typedef struct _listobject {
    OBJECT_VAR_HEAD
    AiObject **ob_item;
    ssize_t allocated;
}
AiListObject;

#define CHECK_TYPE_LIST(ob) CHECK_TYPE(ob, &type_listobject)

#define NUMBER_FREE_LISTS_MAX 80

#define LIST_SIZE OB_SIZE

AiAPI_DATA(AiTypeObject) type_listobject;
AiAPI_DATA(AiListObject *) free_lists[NUMBER_FREE_LISTS_MAX];
AiAPI_DATA(int) number_free_lists;
AiAPI_FUNC(AiObject *) list_new(ssize_t size);
AiAPI_FUNC(void) list_resize(AiListObject *list, ssize_t newsize);
AiAPI_FUNC(AiObject *) list_getitem(AiListObject *list, ssize_t index);
AiAPI_FUNC(AiObject *) list_setitem(AiListObject *list, ssize_t index, AiObject *newitem);
AiAPI_FUNC(AiObject *) list_slice(AiListObject *list, ssize_t start, ssize_t end);
AiAPI_FUNC(int) list_contains(AiListObject *list, AiObject *item);
AiAPI_FUNC(AiObject *) list_to_string(AiListObject *list);
AiAPI_FUNC(int) list_insert(AiListObject *list, ssize_t index, AiObject *item);
AiAPI_FUNC(int) list_append(AiListObject *list, AiObject *item);
AiAPI_FUNC(int) list_extend(AiListObject *fo, AiListObject *la);

#endif
