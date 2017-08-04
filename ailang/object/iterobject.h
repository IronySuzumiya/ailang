#pragma once
#ifndef ITER_OBJECT_H
#define ITER_OBJECT_H

#include "../aiconfig.h"

typedef struct _iterobject {
    AiObject_HEAD
    long it_index;
    AiListObject *it_seq;
}
AiIterObject;

#define CHECK_TYPE_ITER(ob) CHECK_TYPE(ob, &type_iterobject)

AiAPI_DATA(AiTypeObject) type_iterobject;
AiAPI_FUNC(AiObject *) iter_new(AiObject *seq);
AiAPI_FUNC(AiObject *) iter_iternext(AiIterObject *iter);

#endif
