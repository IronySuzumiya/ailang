#include "../ailang.h"

static void none_print(AiNoneObject *ob, FILE *stream);

AiTypeObject type_noneobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "none",                         /* tp_name */
    0,                              /* tp_dealloc */
    (printfunc)none_print,          /* tp_print */
    0,                              /* tp_compare */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    (hashfunc)pointer_hash,         /* tp_hash */
    0,                              /* tp_to_string */
    0,                              /* tp_free */
};

AiNoneObject _none = {
    INIT_OBJECT_HEAD(&type_noneobject)
};

AiNoneObject *none = &_none;

long pointer_hash(void *p) {
    long x;
    size_t y = (size_t)p;
    y = (y >> 4) | (y << (8 * sizeof(void *) - 4));
    x = (long)y;
    return x == -1 ? -2 : x;
}

int object_rich_compare(AiObject *lhs, AiObject *rhs, int op) {
    if (OB_TYPE(lhs) == OB_TYPE(rhs)) {
        switch (op) {
        case CMP_EQ:
            return lhs == rhs || lhs->ob_type->tp_compare(lhs, rhs) == 0;
        case CMP_NE:
            return lhs->ob_type->tp_compare(lhs, rhs) != 0;
        case CMP_GT:
            return lhs->ob_type->tp_compare(lhs, rhs) > 0;
        case CMP_LT:
            return lhs->ob_type->tp_compare(lhs, rhs) < 0;
        case CMP_GE:
            return lhs->ob_type->tp_compare(lhs, rhs) >= 0;
        case CMP_LE:
            return lhs->ob_type->tp_compare(lhs, rhs) <= 0;
        default:
            FATAL_ERROR("internal error: invalid compare option");
            return -1;
        }
    }
    else {
        return -1;
    }
}

AiObject *object_rich_compare_bool(AiObject *lhs, AiObject *rhs, int op) {
    int r = object_rich_compare(lhs, rhs, op);
    return r > 0 ? AiTRUE : r == 0 ? AiFALSE : NONE;
}

void none_print(AiNoneObject *ob, FILE *stream) {

}
