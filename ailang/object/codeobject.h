#pragma once
#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include "../system/utils.h"

typedef struct _codeobject {
    OBJECT_HEAD
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

#define CHECK_TYPE_CODE(ob) CHECK_TYPE(ob, &type_codeobject)

AiAPI_DATA(struct _typeobject) type_codeobject;
AiAPI_FUNC(AiCodeObject *)
code_new(int argcount, int nlocals, int stacksize, int flags,
    AiObject *code, AiObject *consts, AiObject *names,
    AiObject *varnames, AiObject *freevars, AiObject *cellvars,
    AiObject *filename, AiObject *name, int firstlineno,
    AiObject *lnotab);

#endif