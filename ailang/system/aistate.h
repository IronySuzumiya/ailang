#pragma once
#ifndef AI_STATE_H
#define AI_STATE_H

#include "../system/utils.h"

typedef struct _is {
    struct _is *next;
    struct _ts *tstate_head;

    AiObject *modules;
    AiObject *sysdict;
    AiObject *builtins;
}
AiInterpreterState;

typedef struct _ts {
    struct _ts *next;
    struct _is *interp;
    struct _frameobject *frame;
    int recursion_depth;

    AiObject *curexc_type;
    AiObject *curexc_value;
    AiObject *curexc_traceback;

    AiObject *exc_type;
    AiObject *exc_value;
    AiObject *exc_traceback;

    AiObject *dict;
    long thread_id;
}
AiThreadState;

AiAPI_DATA(AiThreadState *) threadstate_current;
AiAPI_FUNC(AiInterpreterState *) interpreterstate_new();
AiAPI_FUNC(void) interpreterstate_clear(AiInterpreterState *interp);
AiAPI_FUNC(void) interpreterstate_delete(AiInterpreterState *interp);
AiAPI_FUNC(AiThreadState *) threadstate_new(AiInterpreterState *interp);
AiAPI_FUNC(void) threadstate_clear(AiThreadState *tstate);
AiAPI_FUNC(void) threadstate_delete(AiThreadState *tstate);
AiAPI_FUNC(AiThreadState *) threadstate_get();
AiAPI_FUNC(AiThreadState *) threadstate_swap(AiThreadState *newts);
AiAPI_FUNC(AiObject *) threadstate_getdict();
AiAPI_FUNC(AiObject *) threadstate_current_frame();

#endif
