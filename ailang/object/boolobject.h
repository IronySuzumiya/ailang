#pragma once
#ifndef BOOL_OBJECT
#define BOOL_OBJECT

#include "../system/utils.h"

#define CHECK_TYPE_BOOL(ob) ((ob) == aitrue || (ob) == aifalse)

#define AiTRUE     ((AiObject *)aitrue)
#define AiFALSE    ((AiObject *)aifalse)

typedef AiIntObject AiBoolObject;

AiAPI_DATA(AiTypeObject) type_boolobject;
AiAPI_DATA(AiBoolObject *) aitrue;
AiAPI_DATA(AiBoolObject *) aifalse;

#endif
