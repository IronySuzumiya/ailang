#pragma once
#ifndef INT_OBJECT_H
#define INT_OBJECT_H

#include "../aiconfig.h"

#define NUM_POS_SMALL_INTOBJECT         257
#define NUM_NEG_SMALL_INTOBJECT         7
#define SMALL_INTOBJECT_BUF_SIZE        (NUM_NEG_SMALL_INTOBJECT + NUM_POS_SMALL_INTOBJECT)
#define IS_SMALL_INTOBJECT(x)           ((x) >= -NUM_NEG_SMALL_INTOBJECT && (x) < NUM_POS_SMALL_INTOBJECT)
#define SMALL_INTOBJECT_INDEX(x)        ((x) + NUM_NEG_SMALL_INTOBJECT)

typedef struct _intobject {
    AiObject_HEAD
    long ob_ival;
}
AiIntObject;

#define NUMBER_INTOBJECT_PER_BLOCK      82
#define INT_BLOCK_SIZE                  (sizeof(struct _intblock *) + sizeof(AiIntObject) * NUMBER_INTOBJECT_PER_BLOCK)
#define INT_BLOCK_HEAD_SIZE             (sizeof(struct _intblock *))

typedef struct _intblock {
    struct _intblock *next;
    AiIntObject block[NUMBER_INTOBJECT_PER_BLOCK];
}
AiIntBlock;

#define CHECK_TYPE_INT(a) CHECK_FAST_SUBCLASS(a, SUBCLASS_INT)
#define CHECK_EXACT_TYPE_INT(a) CHECK_TYPE(a, &AiType_Int)

#define INT_UNARY_WITH_CHECK(ob, op)                                \
    (CHECK_TYPE_INT(ob) ? AiInt_From_Long(op (ob)->ob_ival) : NULL)

#define INT_BINARY_WITH_CHECK(lhs, rhs, op)                         \
    (CHECK_TYPE_INT(lhs) && CHECK_TYPE_INT(rhs) ?                   \
        AiInt_From_Long((lhs)->ob_ival op (rhs)->ob_ival) : NULL)

#define INT_TO_CSTRING_BUFFER_SIZE 33

#define INT_AS_CLONG(ob) (((AiIntObject *)(ob))->ob_ival)

AiAPI_DATA(AiTypeObject) AiType_Int;
AiAPI_FUNC(AiObject *) AiInt_From_Long(long ival);
AiAPI_FUNC(int) AiInt_Init(void);

AiAPI_DATA(int) AiInt_ClearAllMemory(void);

#endif
