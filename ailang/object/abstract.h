#pragma once
#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "../system/utils.h"

#define CMP_EQ          0
#define CMP_NE          1
#define CMP_GT          2
#define CMP_LT          3
#define CMP_GE          4
#define CMP_LE          5
#define CMP_IS          6
#define CMP_IS_NOT      7
#define CMP_IN          8
#define CMP_NOT_IN      9
#define CMP_EXC_MATCH   10

#define CHECK_SEQUENCE(ob)          \
    ((ob)->ob_type->tp_as_sequence  \
    && (ob)->ob_type->tp_as_sequence->sq_length)

#define CHECK_ITERABLE(ob)          \
    (CHECK_SEQUENCE(ob)             \
    && (ob)->ob_type->tp_as_sequence->sq_getitem)

#define SEQUENCE_SIZE(sq)           \
    ((sq)->ob_type->tp_as_sequence->sq_length((AiObject *)sq))

#define SEQUENCE_GETITEM(sq, i)     \
    ((sq)->ob_type->tp_as_sequence->sq_getitem(((AiObject *)(sq)), i))

AiAPI_FUNC(long) pointer_hash(void *p);
AiAPI_FUNC(int) object_rich_compare(AiObject *lhs, AiObject *rhs, int op);
AiAPI_FUNC(AiObject *) object_rich_compare_bool(AiObject *lhs, AiObject *rhs, int op);
AiAPI_FUNC(AiObject *) object_getiter(AiObject *sq);
AiAPI_FUNC(ssize_t) sequence_size(AiObject *sq);
AiAPI_FUNC(AiObject *) sequence_getitem(AiObject *sq, ssize_t index);

#endif
