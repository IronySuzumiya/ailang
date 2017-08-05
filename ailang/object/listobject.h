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

#define CHECK_EXACT_TYPE_LIST(ob) CHECK_TYPE(ob, &AiType_List)
#define CHECK_TYPE_LIST(ob) CHECK_FAST_SUBCLASS(ob, SUBCLASS_LIST)

#define NUMBER_FREE_LISTS_MAX 80

#define LIST_SIZE OB_SIZE
#define LIST_GETITEM(ob, i) (((AiListObject *)(ob))->ob_item[i])
#define LIST_SETITEM(ob, i, v) (LIST_GETITEM(ob, i) = (v))

AiAPI_DATA(AiTypeObject) AiType_List;
AiAPI_FUNC(AiObject *) AiList_New(ssize_t size);
AiAPI_FUNC(AiObject *) AiList_GetItem(AiListObject *list, ssize_t index);
AiAPI_FUNC(int) AiList_SetItem(AiListObject *list, ssize_t index, AiObject *newitem);
AiAPI_FUNC(int) list_append(AiListObject *list, AiObject *item);

AiAPI_FUNC(int) AiList_ClearAllMemory(void);

#endif
