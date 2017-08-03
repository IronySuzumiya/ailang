#pragma once
#ifndef AI_EVAL_H
#define AI_EVAL_H

#include "../system/utils.h"

AiAPI_FUNC(AiObject *) eval_frame(AiFrameObject *f);
AiAPI_FUNC(AiObject *) eval_code(AiCodeObject *co, AiObject *globals, AiObject *locals,
    AiObject **args, int argcount,
    AiObject **kws, int kwcount,
    AiObject **defs, int defcount,
    AiObject *closure);

#endif
