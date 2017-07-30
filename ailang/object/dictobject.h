#pragma once
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include "../system/utils.h"

typedef struct _dictobject {
    OBJECT_HEAD
}
AiDictObject;

AiAPI_DATA(AiTypeObject) type_dictobject;
AiAPI_FUNC(AiObject *) dict_new(void);
AiAPI_FUNC(AiObject *) dict_getitem(AiObject *dict, AiObject *key);
AiAPI_FUNC(void) dict_setitem(AiObject *dict, AiObject *key, AiObject *val);
AiAPI_FUNC(int) dict_delitem(AiObject *dict, AiObject *key);

#endif
