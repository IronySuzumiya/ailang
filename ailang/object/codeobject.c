#include "../ailang.h"

static void intern_strings(AiObject *tuple);
static int intern_string_constants(AiObject *tuple);

AiTypeObject type_codeobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "code",
};

AiObject *code_new(int argcount, int nlocals, int stacksize, int flags,
    AiObject *code, AiObject *consts, AiObject *names,
    AiObject *varnames, AiObject *freevars, AiObject *cellvars,
    AiObject *filename, AiObject *name, int firstlineno,
    AiObject *lnotab) {

    AiCodeObject *co;

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

    return (AiObject *)co;
}

void intern_strings(AiObject *tuple) {
    ssize_t i = TUPLE_SIZE(tuple);
    while (--i >= 0) {
        string_intern((AiStringObject **)&TUPLE_GETITEM(tuple, i));
    }
}

int intern_string_constants(AiObject *tuple) {
    int modified = 0;
    ssize_t i = TUPLE_SIZE(tuple);
    AiObject *v;

    while (--i >= 0) {
        v = TUPLE_GETITEM(tuple, i);
        if (CHECK_TYPE_STRING(v)) {
            AiObject *w = v;
            string_intern((AiStringObject **)&v);
            if (w != v) {
                TUPLE_SETITEM(tuple, i, v);
                modified = 1;
            }
        }
        else if (CHECK_TYPE_TUPLE(v)) {
            intern_string_constants(v);
        }
    }

    return modified;
}

int code_addr2line(AiCodeObject *co, int addrq) {
    int size = (int)(STRING_LEN(co->co_lnotab) / 2);
    unsigned char *p = (unsigned char*)STRING_AS_CSTRING(co->co_lnotab);
    int line = co->co_firstlineno;
    int addr = 0;
    while (--size >= 0) {
        addr += *p++;
        if (addr > addrq)
            break;
        line += *p++;
    }
    return line;
}
