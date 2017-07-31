#include "../ailang.h"

AiTypeObject type_codeobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
};

AiCodeObject *code_new(int argcount, int nlocals, int stacksize, int flags,
    AiObject *code, AiObject *consts, AiObject *names,
    AiObject *varnames, AiObject *freevars, AiObject *cellvars,
    AiObject *filename, AiObject *name, int firstlineno,
    AiObject *lnotab) {

    AiCodeObject *co;

// TODO
    intern_strings(names);
    intern_strings(varnames);
    intern_strings(freevars);
    intern_strings(cellvars);
    intern_string_constants(consts);

    co = AiObject_GC_NEW(AiCodeObject);
    INIT_OBJECT(co, &type_codeobject);

    co->co_argcount = argcount;
    co->co_nlocals = nlocals;
    co->co_stacksize = stacksize;
    co->co_flags = flags;
    INC_REFCNT(code);
    co->co_code = code;
    INC_REFCNT(consts);
    co->co_consts = consts;
    INC_REFCNT(names);
    co->co_names = names;
    INC_REFCNT(varnames);
    co->co_varnames = varnames;
    INC_REFCNT(freevars);
    co->co_freevars = freevars;
    INC_REFCNT(cellvars);
    co->co_cellvars = cellvars;
    INC_REFCNT(filename);
    co->co_filename = filename;
    INC_REFCNT(name);
    co->co_name = name;
    co->co_firstlineno = firstlineno;
    INC_REFCNT(lnotab);
    co->co_lnotab = lnotab;

    return co;
}
