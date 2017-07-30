#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include "../system/utils.h"

#define OBJECT_HEAD                 \
    ssize_t ob_refcnt;              \
    struct _typeobject *ob_type;

#define OBJECT_VAR_HEAD \
    OBJECT_HEAD         \
    ssize_t ob_size;

#define OB_REFCNT(ob)       (((AiObject*)(ob))->ob_refcnt)
#define OB_TYPE(ob)         (((AiObject*)(ob))->ob_type)
#define OB_SIZE(ob)         (((AiVarObject*)(ob))->ob_size)
#define OB_TYPENAME(ob)     (OB_TYPE(ob)->tp_name)
#define OB_DEALLOC(ob)      (OB_TYPE(ob)->tp_dealloc((AiObject *)(ob)))

#define INIT_OBJECT_HEAD(type)              1, type,
#define INIT_OBJECT_VAR_HEAD(type, size)    INIT_OBJECT_HEAD(type) size,

#define INIT_REFCNT(ob)     (OB_REFCNT(ob) = 1)
#define INC_REFCNT(ob)      (++OB_REFCNT(ob))
#define DEC_REFCNT(ob)              \
    if (--OB_REFCNT(ob) > 0);       \
    else OB_DEALLOC(ob)
#define INC_XREFCNT(ob)             \
    if(!ob);                        \
    else INC_REFCNT(ob)
#define DEC_XREFCNT(ob)             \
    if(!ob);                        \
    else DEC_REFCNT(ob)

#define INIT_OBJECT(ob, type)       \
    WRAP(                           \
        OB_TYPE(ob) = (type);       \
        INIT_REFCNT(ob);            \
    )
#define INIT_OBJECT_VAR(ob, type, size) \
    WRAP(                               \
        INIT_OBJECT(ob, type);          \
        OB_SIZE(ob) = (size);           \
    )

#define CHECK_TYPE(ob, type)        \
    ((ob)->ob_type == (type))

#define OB_TO_STRING(ob) ((ob)->ob_type->tp_to_string((AiObject *)(ob)))

#define OB_PRINT_STDOUT(ob) ((ob)->ob_type->tp_print((AiObject *)(ob), stdout))

#define OB_FREE(ob) ((ob)->ob_type->tp_free((ob)))

typedef struct _object {
    OBJECT_HEAD
}
AiObject;

typedef struct _varobject {
    OBJECT_VAR_HEAD
}
AiVarObject;

typedef void(*destructor)(AiObject*);
typedef void(*printfunc)(AiObject*, FILE *);
typedef void(*freefunc)(void *);

typedef int(*enquiry)(AiObject*);
typedef int(*enquiry2)(AiObject*, AiObject*);

typedef AiObject*(*unaryfunc)(AiObject*);
typedef AiObject*(*binaryfunc)(AiObject*, AiObject*);
typedef AiObject*(*ternaryfunc)(AiObject*, AiObject*, AiObject*);

typedef enquiry2 cmpfunc;
typedef ssize_t(*lengthfunc)(AiObject*);
typedef AiObject*(*ssizeargfunc)(AiObject*, ssize_t);
typedef AiObject*(*ssize2argfunc)(AiObject*, ssize_t, ssize_t);
typedef AiObject*(*ssizeobjargfunc)(AiObject*, ssize_t, AiObject*);

typedef long(*hashfunc)(AiObject*);

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
    unaryfunc nb_not;
    binaryfunc nb_xor;

    unaryfunc nb_int;
    unaryfunc nb_long;
    unaryfunc nb_float;
}
numbermethods;

typedef struct _sequencemethods {
    lengthfunc sq_length;
    binaryfunc sq_concat;
    ssizeargfunc sq_getitem;
    ssizeobjargfunc sq_setitem;
    ssize2argfunc sq_slice;
    enquiry2 sq_contains;
}
sequencemethods;

typedef struct _mappingmethods {
    lengthfunc mp_length;
    binaryfunc mp_getitem;
    ternaryfunc mp_setitem;
    enquiry2 mp_delitem;
}
mappingmethods;

typedef struct {
    OBJECT_HEAD
}
AiNoneObject;

#define CHECK_TYPE_NONE(a) CHECK_TYPE(a, &type_noneobject)

#define NONE ((AiObject *)none)

AiAPI_DATA(struct _typeobject) type_noneobject;
AiAPI_DATA(AiNoneObject *) none;
AiAPI_FUNC(long) pointer_hash(void *p);

#endif
