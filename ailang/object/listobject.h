#pragma once
#ifndef LIST_OBJECT
#define LIST_OBJECT

#include "../aiconfig.h"

typedef struct _listobject {
    AiVarObject_HEAD
    AiObject **ob_item;
    ssize_t allocated;
}
AiListObject;

#define CHECK_EXACT_TYPE_LIST(ob) CHECK_TYPE(ob, &type_listobject)
#define CHECK_TYPE_LIST(ob) CHECK_FAST_SUBCLASS(ob, SUBCLASS_LIST)

#define NUMBER_FREE_LISTS_MAX 80

#define LIST_SIZE OB_SIZE
#define LIST_GETITEM(ob, i) (((AiListObject *)(ob))->ob_item[i])
#define LIST_SETITEM(ob, i, v) (LIST_GETITEM(ob, i) = (v))

AiAPI_DATA(AiTypeObject) type_listobject;
AiAPI_FUNC(AiObject *) list_new(ssize_t size);
AiAPI_FUNC(int) list_resize(AiListObject *list, ssize_t newsize);
AiAPI_FUNC(AiObject *) list_getitem(AiListObject *list, ssize_t index);
AiAPI_FUNC(int) list_setitem(AiListObject *list, ssize_t index, AiObject *newitem);
AiAPI_FUNC(AiObject *) list_slice(AiListObject *list, ssize_t start, ssize_t end);
AiAPI_FUNC(int) list_contains(AiListObject *list, AiObject *item);
AiAPI_FUNC(AiObject *) list_str(AiListObject *list);
AiAPI_FUNC(int) list_insert(AiListObject *list, ssize_t index, AiObject *item);
AiAPI_FUNC(int) list_append(AiListObject *list, AiObject *item);
AiAPI_FUNC(int) list_extend(AiListObject *fo, AiListObject *la);

AiAPI_FUNC(int) list_clear_free_lists(void);

#endif
