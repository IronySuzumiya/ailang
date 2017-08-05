#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include "../aiconfig.h"

#define AiObject_HEAD               \
    ssize_t ob_refcnt;              \
    struct _typeobject *ob_type;

#define AiVarObject_HEAD    \
    AiObject_HEAD           \
    ssize_t ob_size;

#define OB_REFCNT(ob)       (((AiObject*)(ob))->ob_refcnt)
#define OB_TYPE(ob)         (((AiObject*)(ob))->ob_type)
#define OB_SIZE(ob)         (((AiVarObject*)(ob))->ob_size)
#define OB_TYPENAME(ob)     (OB_TYPE(ob)->tp_name)
#define OB_DEALLOC(ob)      (OB_TYPE(ob)->tp_dealloc((AiObject *)(ob)))

#define AiObject_HEAD_INIT(type)            1, type,
#define AiVarObject_HEAD_INIT(type, size)   AiObject_HEAD_INIT(type) size,

#define INIT_REFCNT(ob)     (OB_REFCNT(ob) = 1)
#define INC_REFCNT(ob)      (++OB_REFCNT(ob))
#define DEC_REFCNT(ob)              \
    if (--OB_REFCNT(ob) > 0);       \
    else OB_DEALLOC(ob)
#define XINC_REFCNT(ob)             \
    if(!ob);                        \
    else INC_REFCNT(ob)
#define XDEC_REFCNT(ob)             \
    if(!ob);                        \
    else DEC_REFCNT(ob)

#define CHECK_TYPE(ob, type)        \
    ((ob)->ob_type == (type))

#define OB_TO_STRING(ob) ((ob)->ob_type->tp_str((AiObject *)(ob)))
#define OB_PRINT(ob, stream) ((ob)->ob_type->tp_print((AiObject *)(ob), (stream)))
#define OB_PRINT_STDOUT(ob) (OB_PRINT(ob, stdout), fputc('\n', stdout))
#define OB_FREE(ob) ((ob)->ob_type->tp_free((ob)))
#define OB_CLEAR(ob) WRAP(DEC_REFCNT(ob); (ob) = NULL;)

typedef struct _object {
    AiObject_HEAD
}
AiObject;

typedef struct _varobject {
    AiVarObject_HEAD
}
AiVarObject;

typedef AiObject*(*unaryfunc)(AiObject*);
typedef AiObject*(*binaryfunc)(AiObject*, AiObject*);
typedef AiObject*(*ternaryfunc)(AiObject*, AiObject*, AiObject*);

typedef void(*destructor)(AiObject*);
typedef void(*printfunc)(AiObject*, FILE *);
typedef void(*freefunc)(void *);
typedef AiObject *(*getattrfunc)(AiObject *, char *);
typedef AiObject *(*getattrofunc)(AiObject *, AiObject *);
typedef int(*setattrfunc)(AiObject *, char *, AiObject *);
typedef int(*setattrofunc)(AiObject *, AiObject *, AiObject *);

typedef long(*hashfunc)(AiObject*);
typedef int(*sqsetitemfunc)(AiObject*, ssize_t, AiObject*);
typedef int(*mpsetitemfunc)(AiObject*, AiObject*, AiObject*);

typedef AiObject*(*ssizeargfunc)(AiObject*, ssize_t);
typedef AiObject*(*ssize2argfunc)(AiObject*, ssize_t, ssize_t);
typedef AiObject*(*ssizeobjargfunc)(AiObject*, ssize_t, AiObject*);
typedef int(*enquiry)(AiObject*);
typedef int(*enquiry2)(AiObject*, AiObject*);
typedef enquiry2 cmpfunc;
typedef ssize_t(*lengthfunc)(AiObject*);

typedef AiObject *(*descrgetfunc)(AiObject *, AiObject *, AiObject *);
typedef int(*descrsetfunc)(AiObject *, AiObject *, AiObject *);
typedef int(*initproc)(AiObject *, AiObject *, AiObject *);
typedef AiObject *(*newfunc)(struct _typeobject *, AiObject *, AiObject *);
typedef AiObject *(*allocfunc)(struct _typeobject *, ssize_t);

typedef struct _numbermethods {
    binaryfunc nb_add;
    binaryfunc nb_subtract;
    binaryfunc nb_multiply;
    binaryfunc nb_divide;
    binaryfunc nb_modulo;
    binaryfunc nb_power;

    unaryfunc nb_positive;
    unaryfunc nb_negative;
    unaryfunc nb_absolute;

    enquiry nb_nonzero;

    binaryfunc nb_shl;
    binaryfunc nb_shr;
    binaryfunc nb_and;
    binaryfunc nb_or;
    binaryfunc nb_xor;
    unaryfunc nb_not;
    unaryfunc nb_invert;

    unaryfunc nb_int;
    unaryfunc nb_float;
}
AiNumberMethods;

typedef struct _sequencemethods {
    lengthfunc sq_length;
    binaryfunc sq_concat;
    ssizeargfunc sq_getitem;
    sqsetitemfunc sq_setitem;
    ssize2argfunc sq_slice;
    enquiry2 sq_contains;
}
AiSequenceMethods;

typedef struct _mappingmethods {
    lengthfunc mp_length;
    binaryfunc mp_getitem;
    mpsetitemfunc mp_setitem;
    enquiry2 mp_delitem;
}
AiMappingMethods;

#define OBJECT_HASH(ob)                                 \
    ((ob)->ob_type->tp_hash ?                           \
        (ob)->ob_type->tp_hash((AiObject *)(ob)) : -1)

typedef struct _typeobject {
    AiVarObject_HEAD
    char *tp_name;
    ssize_t tp_basicsize;
    ssize_t tp_itemsize;

    destructor tp_dealloc;
    printfunc tp_print;
    cmpfunc tp_compare;

    AiNumberMethods *tp_as_number;
    AiSequenceMethods *tp_as_sequence;
    AiMappingMethods *tp_as_mapping;

    hashfunc tp_hash;
    ternaryfunc tp_call;
    unaryfunc tp_str;

    getattrfunc tp_getattr;
    setattrfunc tp_setattr;
    getattrofunc tp_getattro;
    setattrofunc tp_setattro;

    long tp_flags;

    unaryfunc tp_iter;
    unaryfunc tp_iternext;

    struct _methoddef *tp_methods;
    struct _memberdef *tp_members;
    struct _getsetdef *tp_getset;
    struct _typeobject *tp_base;
    AiObject *tp_dict;
    descrgetfunc tp_descr_get;
    descrsetfunc tp_descr_set;
    ssize_t tp_dictoffset;
    initproc tp_init;
    allocfunc tp_alloc;
    newfunc tp_new;
    freefunc tp_free;
    enquiry tp_is_gc;
    AiObject *tp_bases;
    AiObject *tp_mro;
    AiObject *tp_cache;
    AiObject *tp_subclasses;
    destructor tp_del;
}
AiTypeObject;

typedef struct _heaptypeobject {
    AiTypeObject ht_type;
    AiNumberMethods as_number;
    AiMappingMethods as_mapping;
    AiSequenceMethods as_sequence;
    AiObject *ht_name;
    AiObject *ht_slots;
}
AiHeapTypeObject;

#define CHECK_TYPE_NONE(a) CHECK_TYPE(a, &AiType_None)
#define NONE (&none)
#define GET_NONE() (INC_REFCNT(&none), NONE)

#define AiNotImplemented (&notimplemented)
#define GET_AiNotImplemented() (INC_REFCNT(&notimplemented), NONE)

AiAPI_DATA(AiTypeObject) AiType_None;
AiAPI_DATA(AiTypeObject) AiType_NotImplemented;
AiAPI_DATA(AiTypeObject) AiType_BaseObject;
AiAPI_DATA(AiObject) none;
AiAPI_DATA(AiObject) notimplemented;

#define SUBCLASS_INT        (1L<<0)
#define SUBCLASS_FLOAT      (1L<<1)
#define SUBCLASS_LIST       (1L<<2)
#define SUBCLASS_TUPLE      (1L<<3)
#define SUBCLASS_STRING     (1L<<4)
#define SUBCLASS_DICT       (1L<<5)
#define SUBCLASS_BASEEXC    (1L<<6)
#define SUBCLASS_TYPE       (1L<<7)

#define BASE_TYPE           (1L<<8)
#define HEAP_TYPE           (1L<<9)

#define CHECK_FAST_SUBCLASS(ob, base) ((ob)->ob_type->tp_flags & (base))
#define CHECK_TYPE_TYPE(a) CHECK_FAST_SUBCLASS(a, SUBCLASS_TYPE)

AiAPI_DATA(AiTypeObject) AiType_Type;
AiAPI_FUNC(int) AiType_Ready(AiTypeObject *type);
AiAPI_FUNC(AiObject *) AiType_Generic_Alloc(AiTypeObject *type, ssize_t nitems);

#endif
