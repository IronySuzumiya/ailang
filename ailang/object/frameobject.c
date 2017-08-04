#include "../ailang.h"

static void frame_dealloc(AiFrameObject *f);

// TODO
static AiObject *builtin_object;

AiTypeObject type_frameobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "frame",                            /* tp_name */
    sizeof(AiFrameObject),              /* tp_basicsize */
    sizeof(AiObject *),                 /* tp_itemsize */
    (destructor)frame_dealloc,          /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//object_generic_getattr,             /* tp_getattro */
    0,//object_generic_setattr,             /* tp_setattro */

    0,                                  /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,//frame_methods,                      /* tp_methods */
    0,//frame_memberlist,                   /* tp_members */
    0,//frame_getsetlist,                   /* tp_getset */
};

AiFrameObject *frame_new(AiThreadState *tstate, AiCodeObject *code,
    AiObject *globals, AiObject *locals) {
    AiFrameObject *back = tstate->frame;
    AiFrameObject *f;
    AiObject *builtins;
    ssize_t extras, ncells, nfrees;

    if (!back || back->f_globals != globals) {
        builtins = dict_getitem((AiDictObject *)globals, builtin_object);
        if (builtins) {
            if (CHECK_TYPE_MODULE(builtins)) {
                builtins = module_getdict((AiModuleObject *)builtins);
                INC_REFCNT(builtins);
            }
            else if (!CHECK_TYPE_DICT(builtins)) {
                builtins = dict_new();
            }
        }
        else {
            builtins = dict_new();
        }
    }
    else {
        builtins = back->f_builtins;
        INC_REFCNT(builtins);
    }

    ncells = TUPLE_SIZE(code->co_cellvars);
    nfrees = TUPLE_SIZE(code->co_freevars);

    extras = code->co_stacksize + code->co_nlocals + ncells + nfrees;
    f = AiMEM_ALLOC(sizeof(AiFrameObject) + extras - 1);
    INIT_AiVarObject(f, &type_frameobject, extras);

    f->f_code = code;
    INC_REFCNT(code);
    extras = code->co_nlocals + ncells + nfrees;
    f->f_valuestack = f->f_localsplus + extras;
    AiMEM_SET(f->f_localsplus, 0, sizeof(AiObject *) * extras);
    f->f_locals = NULL;
    f->f_stacktop = f->f_valuestack;
    f->f_builtins = builtins;
    f->f_back = back;
    XINC_REFCNT(back);
    f->f_globals = globals;
    INC_REFCNT(globals);
    
    if (code->co_flags & CO_NEWLOCALS) {
        locals = dict_new();
        f->f_locals = locals;
    }
    else {
        if (!locals) {
            locals = globals;
        }
        INC_REFCNT(locals);
        f->f_locals = locals;
    }
    f->f_tstate = tstate;

    f->f_lasti = -1;
    f->f_lineno = code->co_firstlineno;
    f->f_iblock = 0;
    
    return f;
}

void frame_setup_block(AiFrameObject *f, int type, int handler, int level) {
    AiTryBlock *b;
    b = &f->f_blockstack[f->f_iblock++];
    b->b_type = type;
    b->b_level = level;
    b->b_handler = handler;
}

AiTryBlock *frame_pop_block(AiFrameObject *f) {
    return &f->f_blockstack[--f->f_iblock];
}

AiTryBlock *frame_peek_block(AiFrameObject *f) {
    return &f->f_blockstack[f->f_iblock];
}

void frame_dealloc(AiFrameObject *f) {
    AiObject **p, **valuestack;

    valuestack = f->f_valuestack;
    for (p = f->f_localsplus; p < valuestack; ++p) {
        OB_CLEAR(*p);
    }
    if (f->f_stacktop) {
        for (p = valuestack; p < f->f_stacktop; ++p) {
            XDEC_REFCNT(*p);
        }
    }

    XDEC_REFCNT(f->f_back);
    DEC_REFCNT(f->f_builtins);
    DEC_REFCNT(f->f_globals);
    OB_CLEAR(f->f_locals);
    DEC_REFCNT(f->f_code);

    AiObject_GC_DEL(f);
}
