#pragma once
#ifndef METHOD_H
#define METHOD_H

#include "../aiconfig.h"

typedef AiObject *(*AiCFunction)(AiObject *, AiObject *);
typedef AiObject *(*AiCFunctionWithKeywords)(AiObject *, AiObject *, AiObject *);
typedef AiObject *(*AiNoArgsFunction)(AiObject *);

typedef struct _methoddef {
    char *ml_name;
    AiCFunction ml_meth;
    int ml_flags;
}
AiMethodDef;

#endif
