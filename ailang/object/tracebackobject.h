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

#define CHECK_TYPE_TRACEBACK(ob) CHECK_TYPE(ob, &type_tracebackobject)

AiAPI_DATA(AiTypeObject) type_tracebackobject;
AiAPI_FUNC(void) traceback_here(AiFrameObject *frame);
AiAPI_FUNC(AiTracebackObject *) traceback_new(AiTracebackObject *next, AiFrameObject *frame);

#endif
