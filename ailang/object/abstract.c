#include "../ailang.h"

long pointer_hash(void *p) {
    long x;
    size_t y = (size_t)p;
    y = (y >> 4) | (y << (8 * sizeof(void *) - 4));
    x = (long)y;
    return x == -1 ? -2 : x;
}

long object_hash(AiObject *ob) {
    if (ob->ob_type->tp_hash) {
        return ob->ob_type->tp_hash(ob);
    }
    else {
        return -1;
    }
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
        case CMP_IS:
            return lhs == rhs;
        case CMP_IS_NOT:
            return lhs != rhs;
        case CMP_IN:
            return sequence_contains(rhs, lhs);
        case CMP_NOT_IN:
            return !sequence_contains(rhs, lhs);
        case CMP_EXC_MATCH:
            return exception_matches(lhs, rhs);
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
    return r > 0 ? GET_TRUE() : r == 0 ? GET_FALSE() : NULL;
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

int sequence_contains(AiObject *sq, AiObject *item) {
    if (CHECK_SEQUENCE(sq)) {
        return sq->ob_type->tp_as_sequence->sq_contains(sq, item);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return -1;
    }
}
