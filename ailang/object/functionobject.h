#pragma once
#ifndef FUNCTION_OBJECT_H
#define FUNCTION_OBJECT_H

#include "../system/utils.h"

typedef struct _functionobject {
    OBJECT_HEAD
    AiObject *func_code;
    AiObject *func_globals;
    AiObject *func_defaults;
    AiObject *func_closure;
    AiObject *func_doc;
    AiObject *func_name;
    AiObject *func_dict;
    AiObject *func_weakreflist;
    AiObject *func_module;
}
AiFunctionObject;

#define CHECK_TYPE_FUNCTION(ob) CHECK_TYPE(ob, &type_functionobject)

AiAPI_DATA(AiTypeObject) type_functionobject;
AiAPI_FUNC(AiObject *) function_new(AiObject *code, AiObject *globals);

#endif
