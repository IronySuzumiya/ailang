#include "../ailang.h"

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
    return r > 0 ? AiTRUE : r == 0 ? AiFALSE : NULL;
}

AiObject *object_getiter(AiObject *sq) {
    if (CHECK_ITERABLE(sq)) {
        return iter_new(sq);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return NULL;
    }
}

ssize_t sequence_size(AiObject *sq) {
    if (CHECK_SEQUENCE(sq)) {
        return SEQUENCE_SIZE(sq);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return -1;
    }
}

AiObject *sequence_getitem(AiObject *sq, ssize_t index) {
    if (CHECK_ITERABLE(sq)) {
        return SEQUENCE_GETITEM(sq, index);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return NULL;
    }
}
