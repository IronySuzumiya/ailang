#pragma once
#ifndef AI_EVAL_H
#define AI_EVAL_H

#include "../aiconfig.h"

AiAPI_FUNC(AiObject *) Eval_Frame(AiFrameObject *f);
AiAPI_FUNC(AiObject *) Eval_Code(AiCodeObject *co, AiObject *globals, AiObject *locals,
    AiObject **args, int argcount,
    AiObject **kws, int kwcount,
    AiObject **defs, int defcount,
    AiObject *closure);

#endif
