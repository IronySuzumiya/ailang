#pragma once
#ifndef INT_OBJECT_H
#define INT_OBJECT_H

#include "../system/utils.h"

#define NUM_POS_SMALL_INTOBJECT         257
#define NUM_NEG_SMALL_INTOBJECT         7
#define SMALL_INTOBJECT_BUF_SIZE        (NUM_NEG_SMALL_INTOBJECT + NUM_POS_SMALL_INTOBJECT)
#define IS_SMALL_INTOBJECT(x)           ((x) >= -NUM_NEG_SMALL_INTOBJECT && (x) < NUM_POS_SMALL_INTOBJECT)
#define SMALL_INTOBJECT_INDEX(x)        ((x) + NUM_NEG_SMALL_INTOBJECT)

typedef struct _intobject {
    OBJECT_HEAD
    long ob_ival;
}
IntObject;

#define NUMBER_INTOBJECT_PER_BLOCK      82
#define INT_BLOCK_SIZE                  (sizeof(struct _intblock *) + sizeof(IntObject) * NUMBER_INTOBJECT_PER_BLOCK)
#define INT_BLOCK_HEAD_SIZE             (sizeof(struct _intblock *))

typedef struct _intblock {
    struct _intblock *next;
    IntObject block[NUMBER_INTOBJECT_PER_BLOCK];
}
IntBlock;

#define CHECK_TYPE_INT(a) CHECK_TYPE(a, &type_intobject)

#define INT_UNARY_WITH_CHECK(ob, op)                                \
    (CHECK_TYPE_INT(ob) ? int_fromlong(op (ob)->ob_ival) : NULL)

#define INT_BINARY_WITH_CHECK(lhs, rhs, op)                         \
    (CHECK_TYPE_INT(lhs) && CHECK_TYPE_INT(rhs) ?                   \
        int_fromlong((lhs)->ob_ival op (rhs)->ob_ival) : NULL)

API_DATA(IntBlock *) block_list;
API_DATA(IntObject *) free_list;
API_DATA(TypeObject) type_intobject;
API_DATA(IntObject *) small_intobject_buf[SMALL_INTOBJECT_BUF_SIZE];
API_FUNC(Object *) int_fromlong(long ival);
API_FUNC(void) int_init(void);

#endif
