#pragma once
#ifndef TRACEBACK_OBJECT_H
#define TRACEBACK_OBJECT_H

#include "../aiconfig.h"

typedef struct _tracebackobject {
    AiObject_HEAD
    struct _tracebackobject *tb_next;
    AiFrameObject *tb_frame;
    int tb_lasti;
    int tb_lineno;
}
AiTracebackObject;

#define CHECK_TYPE_TRACEBACK(ob) CHECK_TYPE(ob, &AiType_Traceback)

AiAPI_DATA(AiTypeObject) AiType_Traceback;
AiAPI_FUNC(void) AiTraceback_Here(AiFrameObject *frame);
AiAPI_FUNC(AiTracebackObject *) AiTraceback_New(AiTracebackObject *next, AiFrameObject *frame);

#endif
