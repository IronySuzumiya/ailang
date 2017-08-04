#include "../ailang.h"

static AiObject *bool_and(AiBoolObject *lhs, AiBoolObject *rhs);
static AiObject *bool_or(AiBoolObject *lhs, AiBoolObject *rhs);
static AiObject *bool_not(AiBoolObject *ob);
static void bool_print(AiBoolObject *ob, FILE *stream);
static AiObject *bool_str(AiBoolObject *ob);

static AiBoolObject _aitrue = {
    INIT_AiObject_HEAD(&type_boolobject)
    1
};

static AiBoolObject _aifalse = {
    INIT_AiObject_HEAD(&type_boolobject)
    0
};

static AiNumberMethods bool_as_number = {
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,

    0,

    0,
    0,
    (binaryfunc)bool_and,
    (binaryfunc)bool_or,
    0,
    (unaryfunc)bool_not,
    0,

    0,
    0,
};

AiTypeObject type_boolobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "bool",                             /* tp_name */
    sizeof(AiBoolObject),               /* tp_basesize */
    0,                                  /* tp_itemsize */
    0,                                  /* tp_dealloc */
    (printfunc)bool_print,              /* tp_print */
    0,                                  /* tp_compare */

    &bool_as_number,                    /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    (unaryfunc)bool_str,                /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */

    SUBCLASS_INT | BASE_TYPE,           /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    &type_intobject,                    /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,//bool_new,                           /* tp_new */
    0,                                  /* tp_free */
};

AiBoolObject *aitrue = &_aitrue;
AiBoolObject *aifalse = &_aifalse;

AiObject *bool_from_clong(long ival) {
    return ival ? GET_TRUE() : GET_FALSE();
}

AiObject *bool_and(AiBoolObject *lhs, AiBoolObject *rhs) {
    if (!CHECK_TYPE_BOOL(lhs) || !CHECK_TYPE_BOOL(rhs)) {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
    return lhs == aitrue && rhs == aitrue ? (AiObject *)aitrue : (AiObject *)aifalse;
}

AiObject *bool_or(AiBoolObject *lhs, AiBoolObject *rhs) {
    if (!CHECK_TYPE_BOOL(lhs) || !CHECK_TYPE_BOOL(rhs)) {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
    return lhs == aitrue || rhs == aitrue ? (AiObject *)aitrue : (AiObject *)aifalse;
}

AiObject *bool_not(AiBoolObject *ob) {
    if (!CHECK_TYPE_BOOL(ob)) {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
    return ob == aitrue ? (AiObject *)aifalse : (AiObject *)aitrue;
}

void bool_print(AiBoolObject *ob, FILE *stream) {
    if (ob == aitrue) {
        fputs("True", stream);
    }
    else if (ob == aifalse) {
        fputs("False", stream);
    }
    else {
        FATAL_ERROR("bad bool handling");
    }
}

AiObject *bool_str(AiBoolObject *ob) {
    if (ob == aitrue) {
        return string_from_cstring("True");
    }
    else if (ob == aifalse) {
        return string_from_cstring("False");
    }
    else {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
}
