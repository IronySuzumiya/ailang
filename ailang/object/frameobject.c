#include "../ailang.h"

// TODO
static AiObject *builtin_object;

AiTypeObject type_frameobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
};

AiFrameObject *frame_new(AiThreadState *tstate, AiCodeObject *code,
    AiObject *globals, AiObject *locals) {
    AiFrameObject *back = tstate->frame;
    AiFrameObject *f;
    AiObject *builtins;
    ssize_t extras, ncells, nfrees;

    if (!back || back->f_globals != globals) {
        builtins = dict_getitem((AiDictObject *)globals, builtin_object);
        // TODO
        INC_REFCNT(builtins);
    }
    else {
        builtins = back->f_builtins;
        INC_REFCNT(builtins);
    }

    ncells = TUPLE_SIZE(code->co_cellvars);
    nfrees = TUPLE_SIZE(code->co_freevars);

    extras = code->co_stacksize + code->co_nlocals + ncells + nfrees;
    f = AiMEM_ALLOC(sizeof(AiFrameObject) + extras - 1);
    INIT_OBJECT_VAR(f, &type_frameobject, extras);

    f->f_code = code;
    extras = code->co_nlocals + ncells + nfrees;
    f->f_valuestack = f->f_localsplus + extras;
    AiMEM_SET(f->f_localsplus, 0, sizeof(AiObject *) * extras);
    f->f_locals = NULL;
    f->f_stacktop = f->f_valuestack;
    f->f_builtins = builtins;
    XINC_REFCNT(back);
    f->f_back = back;
    INC_REFCNT(code);
    INC_REFCNT(globals);
    f->f_globals = globals;
    
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
