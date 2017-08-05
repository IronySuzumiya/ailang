#pragma once
#ifndef ITER_OBJECT_H
#define ITER_OBJECT_H

#include "../aiconfig.h"

typedef struct _seqiterobject {
    AiObject_HEAD
    long it_index;
    AiListObject *it_seq;
}
AiSeqiterObject;

#define CHECK_TYPE_ITER(ob) CHECK_TYPE(ob, &AiType_Seqiter)

AiAPI_DATA(AiTypeObject) AiType_Seqiter;
AiAPI_FUNC(AiObject *) AiSeqiter_New(AiObject *seq);

#endif
