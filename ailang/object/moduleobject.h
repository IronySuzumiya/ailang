#pragma once
#ifndef MODULE_OBJECT_H
#define MODULE_OBJECT_H

#include "../aiconfig.h"

typedef struct _moduleobject {
    OBJECT_HEAD
    AiObject *md_dict;
}
AiModuleObject;

#define CHECK_TYPE_MODULE(ob) CHECK_TYPE(ob, &type_moduleobject)

AiAPI_DATA(AiTypeObject) type_moduleobject;
AiAPI_FUNC(AiObject *) module_new(char *name);
AiAPI_FUNC(AiObject *) module_getdict(AiModuleObject *m);

#endif
