#pragma once
#ifndef TUPLE_OBJECT_H
#define TUPLE_OBJECT_H

#include "../aiconfig.h"

typedef struct _tupleobject {
    AiVarObject_HEAD
    AiObject **ob_item;
}
AiTupleObject;

#define CHECK_TYPE_TUPLE(ob) CHECK_TYPE(ob, &type_tupleobject)
#define TUPLE_GETITEM(op, i) (((AiTupleObject *)(op))->ob_item[i])
#define TUPLE_SETITEM(op, i, v) (TUPLE_GETITEM(op, i) = (AiObject *)(v))
#define TUPLE_SIZE OB_SIZE

#define NUMBER_FREE_TUPLES_MAX 20

AiAPI_DATA(AiTypeObject) type_tupleobject;
AiAPI_FUNC(AiObject *) tuple_new(ssize_t size);
AiAPI_FUNC(AiObject *) tuple_getitem(AiTupleObject *tp, ssize_t index);
AiAPI_FUNC(int) tuple_setitem(AiTupleObject *tp, ssize_t index, AiObject *newitem);
AiAPI_FUNC(AiObject *) tuple_slice(AiTupleObject *tp, ssize_t start, ssize_t end);
AiAPI_FUNC(int) tuple_resize(AiTupleObject *tp, ssize_t newsize);
AiAPI_FUNC(AiObject *) tuple_pack(ssize_t argc, ...);
AiAPI_FUNC(ssize_t) tuple_size(AiTupleObject *tp);

#endif
