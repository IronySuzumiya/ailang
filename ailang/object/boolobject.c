#include "../ailang.h"

static AiObject *bool_and(AiBoolObject *lhs, AiBoolObject *rhs);
static AiObject *bool_or(AiBoolObject *lhs, AiBoolObject *rhs);
static AiObject *bool_not(AiBoolObject *ob);
static void bool_print(AiBoolObject *ob, FILE *stream);
static AiObject *bool_to_string(AiBoolObject *ob);

static AiBoolObject _aitrue = {
    INIT_OBJECT_HEAD(&type_boolobject)
    1
};

static AiBoolObject _aifalse = {
    INIT_OBJECT_HEAD(&type_boolobject)
    0
};

static numbermethods bool_as_number = {
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
    (unaryfunc)bool_not,
    0,

    0,
    0,
    0,
};

AiTypeObject type_boolobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "bool",
    0,                              /* tp_dealloc */
    (printfunc)bool_print,          /* tp_print */
    0,                              /* tp_compare */

    &bool_as_number,                /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */

    0,                              /* tp_hash */
    (unaryfunc)bool_to_string,      /* tp_to_string */
    0,                              /* tp_free */
};

AiBoolObject *aitrue = &_aitrue;
AiBoolObject *aifalse = &_aifalse;

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

AiObject *bool_to_string(AiBoolObject *ob) {
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
