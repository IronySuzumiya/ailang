#pragma once
#ifndef AI_STATE_H
#define AI_STATE_H

#include "../aiconfig.h"

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

AiAPI_DATA(AiThreadState *) AiThreadState_Current;
AiAPI_FUNC(AiInterpreterState *) AiInterpreterState_New();
AiAPI_FUNC(void) AiInterpreterState_Clear(AiInterpreterState *interp);
AiAPI_FUNC(void) AiInterpreterState_Delete(AiInterpreterState *interp);
AiAPI_FUNC(AiThreadState *) AiThreadState_New(AiInterpreterState *interp);
AiAPI_FUNC(void) AiThreadState_Clear(AiThreadState *tstate);
AiAPI_FUNC(void) AiThreadState_Delete(AiThreadState *tstate);
AiAPI_FUNC(AiThreadState *) AiThreadState_Get();
AiAPI_FUNC(AiThreadState *) AiThreadState_Swap(AiThreadState *newts);
AiAPI_FUNC(AiObject *) AiThreadState_Getdict();
AiAPI_FUNC(AiObject *) AiThreadState_Current_Frame();

#endif
