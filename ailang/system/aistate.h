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
    AiObject *dict;
    long thread_id;
}
AiThreadState;

#endif
