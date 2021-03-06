#pragma once
#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include "../aiconfig.h"

typedef struct _codeobject {
    AiObject_HEAD
    int co_argcount;
    int co_nlocals;
    int co_stacksize;
    int co_flags;

    AiObject *co_code;
    AiObject *co_consts;
    AiObject *co_names;
    AiObject *co_varnames;
    AiObject *co_freevars;
    AiObject *co_cellvars;

    AiObject *co_filename;
    AiObject *co_name;
    int co_firstlineno;
    AiObject *co_lnotab;
}
AiCodeObject;

#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020
#define CO_NOFREE       0x0040

#define CO_MAXBLOCKS    20

#define CHECK_TYPE_CODE(ob) CHECK_TYPE(ob, &AiType_Code)
#define CODE_GETNUMFREE(ob) (TUPLE_SIZE(((AiCodeObject *)(ob))->co_freevars))

AiAPI_DATA(struct _typeobject) AiType_Code;
AiAPI_FUNC(AiObject *) AiCode_New(int argcount, int nlocals, int stacksize, int flags,
    AiObject *code, AiObject *consts, AiObject *names,
    AiObject *varnames, AiObject *freevars, AiObject *cellvars,
    AiObject *filename, AiObject *name, int firstlineno,
    AiObject *lnotab);
AiAPI_FUNC(int) AiCode_Addr2Line(AiCodeObject *co, int addrq);

#endif
