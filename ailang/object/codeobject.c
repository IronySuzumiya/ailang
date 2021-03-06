#include "../ailang.h"

static void code_dealloc(AiCodeObject *co);
static void intern_strings(AiObject *tuple);
static int intern_string_constants(AiObject *tuple);

AiTypeObject AiType_Code = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "code",                             /* tp_name */
    sizeof(AiCodeObject),               /* tp_basicsize */
    0,                                  /* tp_itemsize */

    (destructor)code_dealloc,           /* tp_dealloc */
    0,                                  /* tp_compare */
    0,//(hashfunc)code_hash,                /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//code_new,                           /* tp_new */
};

AiObject *AiCode_New(int argcount, int nlocals, int stacksize, int flags,
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

    co = AiObject_NEW(AiCodeObject, &AiType_Code);

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

int AiCode_Addr2Line(AiCodeObject *co, int addrq) {
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

void intern_strings(AiObject *tuple) {
    ssize_t i = TUPLE_SIZE(tuple);
    while (--i >= 0) {
        AiString_Intern((AiStringObject **)&TUPLE_GETITEM(tuple, i));
    }
}

int intern_string_constants(AiObject *tuple) {
    int modified = 0;
    ssize_t i = TUPLE_SIZE(tuple);
    AiObject *v;

    while (--i >= 0) {
        v = TUPLE_GETITEM(tuple, i);
        if (CHECK_EXACT_TYPE_STRING(v)) {
            AiObject *w = v;
            AiString_Intern((AiStringObject **)&v);
            if (w != v) {
                TUPLE_SETITEM(tuple, i, v);
                modified = 1;
            }
        }
        else if (CHECK_EXACT_TYPE_TUPLE(v)) {
            intern_string_constants(v);
        }
    }

    return modified;
}

void code_dealloc(AiCodeObject *co) {
    XDEC_REFCNT(co->co_code);
    XDEC_REFCNT(co->co_consts);
    XDEC_REFCNT(co->co_names);
    XDEC_REFCNT(co->co_varnames);
    XDEC_REFCNT(co->co_freevars);
    XDEC_REFCNT(co->co_cellvars);
    XDEC_REFCNT(co->co_filename);
    XDEC_REFCNT(co->co_name);
    XDEC_REFCNT(co->co_lnotab);
    AiObject_Del(co);
}
