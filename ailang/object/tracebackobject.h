#pragma once
#ifndef TRACEBACK_OBJECT_H
#define TRACEBACK_OBJECT_H

#include "../system/utils.h"

typedef struct _tracebackobject {
    OBJECT_HEAD
    struct _tracebackobject *tb_next;
    AiFrameObject *tb_frame;
    int tb_lasti;
    int tb_lineno;
}
AiTracebackObject;

AiAPI_DATA(AiTypeObject) type_tracebackobject;
AiAPI_FUNC(void) traceback_here(AiFrameObject *frame);

#endif
