#include "../ailang.h"

long AiObject_Unhashable(AiObject *p) {
    RUNTIME_EXCEPTION("unhashable type");
    return -1;
}

long Pointer_Hash(void *p) {
    long x;
    size_t y = (size_t)p;
    y = (y >> 4) | (y << (8 * sizeof(void *) - 4));
    x = (long)y;
    return x == -1 ? -2 : x;
}

long AiObject_Generic_Hash(AiObject *ob) {
    if (ob->ob_type->tp_hash) {
        return ob->ob_type->tp_hash(ob);
    }
    else {
        return -1;
    }
}

int AiObject_Rich_Compare(AiObject *lhs, AiObject *rhs, int op) {
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
            return AiSequence_Contains(rhs, lhs);
        case CMP_NOT_IN:
            return !AiSequence_Contains(rhs, lhs);
        case CMP_EXC_MATCH:
            return AiException_Matches(lhs, rhs);
        default:
            FATAL_ERROR("internal error: invalid compare option");
            return -1;
        }
    }
    else {
        return -1;
    }
}

AiObject *AiObject_Rich_Compare_Bool(AiObject *lhs, AiObject *rhs, int op) {
    int r = AiObject_Rich_Compare(lhs, rhs, op);
    return r > 0 ? GET_TRUE() : r == 0 ? GET_FALSE() : NULL;
}

AiObject *AiObject_Generic_Getiter(AiObject *sq) {
    if (CHECK_ITERABLE(sq)) {
        return AiSeqiter_New(sq);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return NULL;
    }
}

ssize_t AiSequence_Getsize(AiObject *sq) {
    if (CHECK_SEQUENCE(sq)) {
        return SEQUENCE_SIZE(sq);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return -1;
    }
}

AiObject *AiSequence_Getitem(AiObject *sq, ssize_t index) {
    if (CHECK_ITERABLE(sq)) {
        return SEQUENCE_GETITEM(sq, index);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return NULL;
    }
}

int AiSequence_Contains(AiObject *sq, AiObject *item) {
    if (CHECK_SEQUENCE(sq)) {
        return sq->ob_type->tp_as_sequence->sq_contains(sq, item);
    }
    else {
        RUNTIME_EXCEPTION("only sequence supported");
        return -1;
    }
}

AiObject *AiObject_Call(AiObject *func, AiObject *arg, AiObject *kw) {
    ternaryfunc call;
    AiObject *result;

    if (call = func->ob_type->tp_call) {
        result = (*call)(func, arg, kw);
        if (!result) {
            RUNTIME_EXCEPTION("NULL result in AiObject_Call");
        }
    }
    else {
        RUNTIME_EXCEPTION("'%s' object is not callable", func->ob_type->tp_name);
    }
    return result;
}
