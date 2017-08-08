#pragma once
#ifndef BOOL_OBJECT
#define BOOL_OBJECT

#include "../aiconfig.h"

#define CHECK_TYPE_BOOL(ob) ((ob) == aitrue || (ob) == aifalse)

#define AiTRUE          ((AiObject *)aitrue)
#define AiFALSE         ((AiObject *)aifalse)
#define GET_TRUE()      (INC_REFCNT(aitrue), AiTRUE)
#define GET_FALSE()     (INC_REFCNT(aifalse), AiFALSE)
#define RETURN_TRUE     return INC_REFCNT(AiTRUE), AiTRUE
#define RETURN_FALSE    return INC_REFCNT(AiFALSE), AiFALSE

typedef AiIntObject AiBoolObject;

AiAPI_DATA(AiTypeObject) AiType_Bool;
AiAPI_DATA(AiBoolObject *) aitrue;
AiAPI_DATA(AiBoolObject *) aifalse;
AiAPI_FUNC(AiObject *) AiBool_From_Long(long ival);
AiAPI_FUNC(int) AiBool_Ready(void);

#endif
