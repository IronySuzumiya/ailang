#pragma once
#ifndef METHOD_H
#define METHOD_H

#include "../aiconfig.h"

#define METH_VARARGS    0x0001
#define METH_KEYWORDS   0x0002
#define METH_NOARGS     0x0004
#define METH_O          0x0008

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
