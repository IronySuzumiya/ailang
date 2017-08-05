#pragma once
#ifndef MODULE_OBJECT_H
#define MODULE_OBJECT_H

#include "../aiconfig.h"

typedef struct _moduleobject {
    AiObject_HEAD
    AiObject *md_dict;
}
AiModuleObject;

#define CHECK_TYPE_MODULE(ob) CHECK_TYPE(ob, &AiType_Module)

AiAPI_DATA(AiTypeObject) AiType_Module;
AiAPI_FUNC(AiObject *) AiModule_New(char *name);
AiAPI_FUNC(AiObject *) AiModule_Getdict(AiModuleObject *m);

#endif
