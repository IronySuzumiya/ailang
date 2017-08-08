#include "../ailang.h"

static void frame_dealloc(AiFrameObject *f);

// TODO
static AiObject *builtin_object;

AiTypeObject AiType_Frame = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "frame",                            /* tp_name */
    sizeof(AiFrameObject),              /* tp_basicsize */
    sizeof(AiObject *),                 /* tp_itemsize */

    (destructor)frame_dealloc,          /* tp_dealloc */
    0,                                  /* tp_compare */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
};

AiFrameObject *AiFrame_New(AiThreadState *tstate, AiCodeObject *code,
    AiObject *globals, AiObject *locals) {
    AiFrameObject *back = tstate->frame;
    AiFrameObject *f;
    AiObject *builtins;
    ssize_t extras, ncells, nfrees;

    if (!back || back->f_globals != globals) {
        builtins = AiDict_GetItem((AiDictObject *)globals, builtin_object);
        if (builtins) {
            if (CHECK_TYPE_MODULE(builtins)) {
                builtins = AiModule_Getdict((AiModuleObject *)builtins);
                INC_REFCNT(builtins);
            }
            else if (!CHECK_EXACT_TYPE_DICT(builtins)) {
                builtins = AiDict_New();
            }
        }
        else {
            builtins = AiDict_New();
        }
    }
    else {
        builtins = back->f_builtins;
        INC_REFCNT(builtins);
    }

    ncells = TUPLE_SIZE(code->co_cellvars);
    nfrees = TUPLE_SIZE(code->co_freevars);

    extras = code->co_stacksize + code->co_nlocals + ncells + nfrees;
    f = AiVarObject_NEW(AiFrameObject, &AiType_Frame, max(extras - 1, 0));

    f->f_code = code;
    INC_REFCNT(code);
    extras = code->co_nlocals + ncells + nfrees;
    f->f_valuestack = f->f_localsplus + extras;
    AiMem_Set(f->f_localsplus, 0, sizeof(AiObject *) * extras);
    f->f_locals = NULL;
    f->f_stacktop = f->f_valuestack;
    f->f_builtins = builtins;
    f->f_back = back;
    XINC_REFCNT(back);
    f->f_globals = globals;
    INC_REFCNT(globals);
    
    if (code->co_flags & CO_NEWLOCALS) {
        locals = AiDict_New();
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

void AiFrame_Setup_Block(AiFrameObject *f, int type, int handler, int level) {
    AiTryBlock *b;
    b = &f->f_blockstack[f->f_iblock++];
    b->b_type = type;
    b->b_level = level;
    b->b_handler = handler;
}

AiTryBlock *AiFrame_Pop_Block(AiFrameObject *f) {
    return &f->f_blockstack[--f->f_iblock];
}

AiTryBlock *AiFrame_Peek_Block(AiFrameObject *f) {
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

    AiObject_Del(f);
}
