#pragma once
#ifndef TUPLE_OBJECT_H
#define TUPLE_OBJECT_H

#include "../aiconfig.h"

typedef struct _tupleobject {
    AiVarObject_HEAD
    AiObject *ob_item[1];
}
AiTupleObject;

#define CHECK_TYPE_TUPLE(ob) CHECK_FAST_SUBCLASS(ob, SUBCLASS_TUPLE)
#define CHECK_EXACT_TYPE_TUPLE(ob) CHECK_TYPE(ob, &AiType_Tuple)
#define TUPLE_GETITEM(op, i) (((AiTupleObject *)(op))->ob_item[i])
#define TUPLE_SETITEM(op, i, v) (TUPLE_GETITEM(op, i) = (AiObject *)(v))
#define TUPLE_SIZE OB_SIZE

#define NUMBER_FREE_TUPLES_MAX 20

AiAPI_DATA(AiTypeObject) AiType_Tuple;
AiAPI_FUNC(AiObject *) AiTuple_New(ssize_t size);
AiAPI_FUNC(AiObject *) AiTuple_GetItem(AiTupleObject *tp, ssize_t index);
AiAPI_FUNC(int) AiTuple_SetItem(AiTupleObject *tp, ssize_t index, AiObject *newitem);
AiAPI_FUNC(AiObject *) AiTuple_Slice(AiTupleObject *tp, ssize_t start, ssize_t end);
AiAPI_FUNC(AiObject *) AiTuple_Pack(ssize_t argc, ...);
AiAPI_FUNC(ssize_t) tuple_size(AiTupleObject *tp);

#endif
