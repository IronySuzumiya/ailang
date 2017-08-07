#pragma once
#ifndef DESCR_OBJECT_H
#define DESCR_OBJECT_H

#include "../aiconfig.h"

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
    AiMemberDef *d_member;
}
AiMemberDescrObject;

typedef struct _wrapperdescrobject {
    AiDescr_HEAD
    struct wrapperbase *d_base;
    void *d_wrapped;
}
AiWrapperDescrObject;

AiAPI_DATA(AiTypeObject) AiType_MethodDescr;
AiAPI_DATA(AiTypeObject) AiType_ClassMethodDescr;
AiAPI_DATA(AiTypeObject) AiType_MemberDescr;
AiAPI_DATA(AiTypeObject) AiType_GetSetDescr;
AiAPI_DATA(AiTypeObject) AiType_WrapperDescr;
AiAPI_FUNC(AiObject *) AiDescr_NewWrapper(AiTypeObject *type, struct wrapperbase *base, void *wrapped);
AiAPI_FUNC(AiObject *) AiDescr_NewMember(AiTypeObject *type, AiMemberDef *member);
AiAPI_FUNC(AiObject *) AiDescr_NewMethod(AiTypeObject *type, AiMethodDef *method);

#endif
