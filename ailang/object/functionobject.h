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

#define FUNCTION_GETCODE(ob) (((AiFunctionObject *)(ob))->func_code)
#define FUNCTION_GETGLOBALS(ob) (((AiFunctionObject *)(ob))->func_globals)
#define FUNCTION_GETDEFAULTS(ob) (((AiFunctionObject *)(ob))->func_defaults)
#define FUNCTION_GETMODULE(ob) (((AiFunctionObject *)(ob))->func_module)
#define FUNCTION_GETCLOSURE(ob) (((AiFunctionObject *)(ob))->func_closure)

#define FUNCTION_SETDEFAULTS(ob, v) (FUNCTION_GETDEFAULTS(ob) = (v))
#define FUNCTION_SETCLOSURE(ob, v) (FUNCTION_GETCLOSURE(ob) = (v))

AiAPI_DATA(AiTypeObject) type_functionobject;
AiAPI_FUNC(AiObject *) function_new(AiObject *code, AiObject *globals);
AiAPI_FUNC(int) function_setdefaults(AiFunctionObject *func, AiObject *defaults);
AiAPI_FUNC(int) function_setclosure(AiFunctionObject *func, AiObject *closure);

#endif
