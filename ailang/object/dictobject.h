#pragma once
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include "../system/utils.h"

typedef struct _dictobject {
    OBJECT_HEAD
}
DictObject;

API_DATA(TypeObject) type_dictobject;
API_FUNC(Object *) dict_new(void);
API_FUNC(Object *) dict_getitem(Object *dict, Object *key);
API_FUNC(void) dict_setitem(Object *dict, Object *key, Object *val);
API_FUNC(int) dict_delitem(Object *dict, Object *key);

#endif
