#pragma once
#ifndef CLASS_OBJECT_H
#define CLASS_OBJECT_H

#include "../aiconfig.h"

typedef struct _classobject {
    AiObject_HEAD
    AiObject *cl_base;
    AiObject *cl_dict;
    AiObject *cl_name;
    AiObject *cl_getattr;
    AiObject *cl_setattr;
    AiObject *cl_delattr;
}
AiClassObject;

typedef struct _instanceobject {
    AiObject_HEAD
    AiClassObject *in_class;
    AiObject *in_dict;
}
AiInstanceObject;

typedef struct _methodobject {
    AiObject_HEAD
    AiObject *im_func;
    AiObject *im_self;
    AiObject *im_class;
}
AiMethodObject;

#define CHECK_TYPE_CLASS(ob) CHECK_TYPE(ob, &AiType_Class)
#define CHECK_TYPE_INSTANCE(ob) CHECK_TYPE(ob, &AiType_Instance)
#define CHECK_TYPE_METHOD(ob) CHECK_TYPE(ob, &AiType_Method)

#define AiMethod_FUNCTION(meth) \
    (((AiMethodObject *)(meth))->im_func)
#define AiMethod_SELF(meth)     \
    (((AiMethodObject *)(meth))->im_self)
#define AiMethod_CLASS(meth)    \
    (((AiMethodObject *)(meth))->im_class)

AiAPI_DATA(AiTypeObject) AiType_Class, AiType_Instance, AiType_Method;
AiAPI_FUNC(AiObject *) AiClass_New(AiObject *, AiObject *, AiObject *);
AiAPI_FUNC(AiObject *) AiInstance_New(AiObject *, AiObject *, AiObject *);
AiAPI_FUNC(AiObject *) AiMethod_New(AiObject *, AiObject *, AiObject *);

#endif
