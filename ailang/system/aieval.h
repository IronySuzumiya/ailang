#pragma once
#ifndef AI_EVAL_H
#define AI_EVAL_H

#include "../system/utils.h"

#define INSTR_OFFSET()      ((int)(next_instr - first_instr))
#define NEXTOP()            (*next_instr++)
#define NEXTARG()           (next_instr += 2, (next_instr[-1]<<8) + next_instr[-2])
#define PEEKARG()           ((next_instr[2]<<8) + next_instr[1])
#define JUMPTO(x)           (next_instr = first_instr + (x))
#define JUMPBY(x)           (next_instr += (x))

#define STACK_LEVEL()       ((int)(stack_pointer - f->f_valuestack))
#define EMPTY()             (STACK_LEVEL() == 0)
#define TOP()               (stack_pointer[-1])
#define SECOND()            (stack_pointer[-2])
#define THIRD()             (stack_pointer[-3])
#define FOURTH()            (stack_pointer[-4])
#define PEEK(n)             (stack_pointer[-(n)])
#define SET_TOP(v)          (stack_pointer[-1] = (v))
#define SET_SECOND(v)       (stack_pointer[-2] = (v))
#define SET_THIRD(v)        (stack_pointer[-3] = (v))
#define SET_FOURTH(v)       (stack_pointer[-4] = (v))
#define SET_VALUE(n, v)     (stack_pointer[-(n)] = (v))
#define BASIC_STACKADJ(n)   (stack_pointer += n)
#define BASIC_PUSH(v)       (*stack_pointer++ = (v))
#define BASIC_POP()         (*--stack_pointer)
#define PUSH                BASIC_PUSH
#define POP                 BASIC_POP
#define STACKADJ            BASIC_STACKADJ

#define GETLOCAL(i)         (fastlocals[i])
#define SETLOCAL(i, value)              \
    WRAP(                               \
        AiObject *tmp = GETLOCAL(i);    \
        GETLOCAL(i) = value;            \
        XDEC_REFCNT(tmp);               \
    )

enum why_code {
    WHY_NOT = 0x0001,       /* No error */
    WHY_EXCEPTION = 0x0002, /* Exception occurred */
    WHY_RERAISE = 0x0004,   /* Exception re-raised by 'finally' */
    WHY_RETURN = 0x0008,    /* 'return' statement */
    WHY_BREAK = 0x0010,     /* 'break' statement */
    WHY_CONTINUE = 0x0020,  /* 'continue' statement */
    WHY_YIELD = 0x0040      /* 'yield' operator */
};

#endif
