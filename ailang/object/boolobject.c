#include "../ailang.h"

static AiObject *bool_and(AiBoolObject *lhs, AiBoolObject *rhs);
static AiObject *bool_or(AiBoolObject *lhs, AiBoolObject *rhs);
static AiObject *bool_not(AiBoolObject *ob);
static AiObject *bool_str(AiBoolObject *ob);
static AiObject *bool_new(AiTypeObject *type, AiObject *args, AiObject *kw);

static AiObject *bool_and_wrapper(AiObject *args, AiObject *kwds);
static AiObject *bool_or_wrapper(AiObject *args, AiObject *kwds);
static AiObject *bool_not_wrapper(AiObject *args, AiObject *kwds);

static AiSlotDef bool_slots[] = {
    { "and", bool_and_wrapper },
    { "or", bool_or_wrapper },
    { "not", bool_not_wrapper },
    { NULL }
};

static AiBoolObject _aitrue = {
    AiObject_HEAD_INIT(&AiType_Bool)
    1
};

static AiBoolObject _aifalse = {
    AiObject_HEAD_INIT(&AiType_Bool)
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

AiTypeObject AiType_Bool = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "bool",                             /* tp_name */
    sizeof(AiBoolObject),               /* tp_basesize */
    0,                                  /* tp_itemsize */

    0,                                  /* tp_dealloc */
    0,                                  /* tp_compare */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    (unaryfunc)bool_str,                /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    &bool_as_number,                    /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    &AiType_Int,                        /* tp_base */
    0,                                  /* tp_dict */
    bool_new,                           /* tp_new */
};

AiBoolObject *aitrue = &_aitrue;
AiBoolObject *aifalse = &_aifalse;

AiObject *AiBool_From_Long(long ival) {
    if (ival)
        RETURN_TRUE;
    else
        RETURN_FALSE;
}

AiObject *AiBool_Ready(void) {
    AiObject *funco;
    AiType_Bool.tp_dict = AiDict_New();
    for (AiSlotDef *slot = bool_slots; slot; ++slot) {
        funco = AiCFunction_New(slot->func);
        AiDict_SetItem((AiDictObject *)AiType_Bool.tp_dict, slot->name, funco);
    }
}

AiObject *bool_and(AiBoolObject *lhs, AiBoolObject *rhs) {
    if (!CHECK_TYPE_BOOL(lhs) || !CHECK_TYPE_BOOL(rhs)) {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
    if (lhs == aitrue && rhs == aitrue)
        RETURN_TRUE;
    else
        RETURN_FALSE;
}

AiObject *bool_or(AiBoolObject *lhs, AiBoolObject *rhs) {
    if (!CHECK_TYPE_BOOL(lhs) || !CHECK_TYPE_BOOL(rhs)) {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
    if (lhs == aitrue || rhs == aitrue)
        RETURN_TRUE;
    else
        RETURN_FALSE;
}

AiObject *bool_not(AiBoolObject *ob) {
    if (!CHECK_TYPE_BOOL(ob)) {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
    if (ob == aitrue)
        RETURN_TRUE;
    else
        RETURN_FALSE;
}

AiObject *bool_str(AiBoolObject *ob) {
    if (ob == aitrue) {
        return AiString_From_String("True");
    }
    else if (ob == aifalse) {
        return AiString_From_String("False");
    }
    else {
        FATAL_ERROR("bad bool handling");
        return NULL;
    }
}

AiObject *bool_new(AiTypeObject *type, AiObject *args, AiObject *kw) {
    AiIntObject *i = TUPLE_GETITEM(args, 0);
    return AiBool_From_Long(i);
}

AiObject *bool_and_wrapper(AiObject *args, AiObject *kwds) {
    AiBoolObject *lhs = TUPLE_GETITEM(args, 0);
    AiBoolObject *rhs = TUPLE_GETITEM(args, 1);
    return bool_and(lhs, rhs);
}

AiObject *bool_or_wrapper(AiObject *args, AiObject *kwds) {
    AiBoolObject *lhs = TUPLE_GETITEM(args, 0);
    AiBoolObject *rhs = TUPLE_GETITEM(args, 1);
    return bool_or(lhs, rhs);
}

AiObject *bool_not_wrapper(AiObject *args, AiObject *kwds) {
    AiBoolObject *lhs = TUPLE_GETITEM(args, 0);
    AiBoolObject *rhs = TUPLE_GETITEM(args, 1);
    return bool_not(lhs, rhs);
}
