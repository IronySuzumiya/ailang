#pragma once
#ifndef FRAME_OBJECT_H
#define FRAME_OBJECT_H

#include "../system/utils.h"

typedef struct _tryblock {
    int b_type;
    int b_handler;
    int b_level;
}
AiTryBlock;

typedef struct _frameobject {
    OBJECT_VAR_HEAD
    struct _frameobject *f_back;
    AiCodeObject *f_code;
    AiObject *f_builtins;
    AiObject *f_globals;
    AiObject *f_locals;
    AiObject **f_valuestack;
    AiObject **f_stacktop;

    struct _ts *f_tstate;
    int f_lasti;
    int f_lineno;
    int f_iblock;
    AiTryBlock f_blockstack[CO_MAXBLOCKS]; /* for try and loop blocks */
    AiObject *f_localsplus[1];
}
AiFrameObject;

#define CHECK_TYPE_FRAME(ob) CHECK_TYPE(ob, &type_frameobject)

AiAPI_DATA(AiTypeObject) type_frameobject;
AiAPI_FUNC(AiFrameObject *) frame_new(AiThreadState *tstate, AiCodeObject *code,
    AiObject *globals, AiObject *locals);
AiAPI_FUNC(void) frame_setup_block(AiFrameObject *f, int type, int handler, int level);
AiAPI_FUNC(AiTryBlock *) frame_pop_block(AiFrameObject *f);

#endif
