#pragma once
#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "../aiconfig.h"

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
    ((sq)->ob_type->tp_as_sequence->sq_length((AiObject *)(sq)))

#define SEQUENCE_GETITEM(sq, i)     \
    ((sq)->ob_type->tp_as_sequence->sq_getitem(((AiObject *)(sq)), i))

AiAPI_FUNC(long) AiObject_Unhashable(AiObject *p);
AiAPI_FUNC(long) Pointer_Hash(void *p);
AiAPI_FUNC(long) AiObject_Generic_Hash(AiObject *ob);
AiAPI_FUNC(int) AiObject_Rich_Compare(AiObject *lhs, AiObject *rhs, int op);
AiAPI_FUNC(AiObject *) AiObject_Rich_Compare_Bool(AiObject *lhs, AiObject *rhs, int op);
AiAPI_FUNC(AiObject *) AiObject_Generic_Getiter(AiObject *sq);
AiAPI_FUNC(ssize_t) AiSequence_Getsize(AiObject *sq);
AiAPI_FUNC(AiObject *) AiSequence_Getitem(AiObject *sq, ssize_t index);
AiAPI_FUNC(int) AiSequence_Contains(AiObject *sq, AiObject *item);

#endif
