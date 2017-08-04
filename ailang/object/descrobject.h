#pragma once
#ifndef DESCR_OBJECT_H
#define DESCR_OBJECT_H

#include "../aiconfig.h"

typedef AiObject *(*getter)(AiObject *, void *);
typedef int(*setter)(AiObject *, AiObject *, void *);

typedef struct _getsetdef {
    char *name;
    getter get;
    setter set;
    void *closure;
}
AiGetSetDef;

typedef AiObject *(*wrapperfunc)(AiObject *self, AiObject *args, void *wrapped);
typedef AiObject *(*wrapperfunc_kwds)(AiObject *self, AiObject *args,
                                        void *wrapped, AiObject *kwds);

struct wrapperbase {
    char *name;
    int offset;
    void *function;
    wrapperfunc wrapper;
    char *doc;
    int flags;
    AiObject *name_strobj;
};

#define WRAPPER_FLAG_KEYWORDS 1

#define AiDescr_HEAD        \
    AiObject_HEAD           \
    AiTypeObject *d_type;   \
    AiObject *d_name;

typedef struct _descrobject {
    AiDescr_HEAD
}
AiDescrObject;

typedef struct _methoddescrobject {
    AiDescr_HEAD
    AiMethodDef *d_method;
}
AiMethodDescrObject;

typedef struct _memberdescrobject {
    AiDescr_HEAD
    AiGetSetDef *d_getset;
}
AiMemberDescrObject;

typedef struct _wrapperdescrobject {
    AiDescr_HEAD
    struct _wrapperbase *d_base;
    void *d_wrapped;
}
AiWrapperDescrObject;

#endif
