#include "../ailang.h"

AiTypeObject type_tupleobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)

};

AiTupleObject *free_tuples[NUMBER_FREE_TUPLES_MAX];
int number_free_tuples[NUMBER_FREE_TUPLES_MAX];

AiObject * tuple_new(ssize_t size) {
    AiTupleObject *ob;
    ssize_t nbytes = size * sizeof(AiObject *);

    if (size == 0 && (ob = free_tuples[0])) {
        INC_REFCNT(ob);
        return (AiObject *)ob;
    }
    else if (size < NUMBER_FREE_TUPLES_MAX && (ob = free_tuples[size])) {
        free_tuples[size] = (AiTupleObject *)ob->ob_item[0];
        --number_free_tuples[size];
        INIT_OBJECT_VAR(ob, &type_tupleobject, size);
    }
    else {
        ob = AiMEM_ALLOC(nbytes);
        INIT_OBJECT_VAR(ob, &type_tupleobject, size);
    }
    AiMEM_SET(ob->ob_item, 0, nbytes);

    if (size == 0) {
        free_tuples[0] = ob;
        ++number_free_tuples[0];
        INC_REFCNT(ob);
    }
    return (AiObject *)ob;
}

AiObject *tuple_getitem(AiTupleObject *tp, ssize_t index) {
    if (index < 0 || index >= TUPLE_SIZE(tp)) {
        RUNTIME_EXCEPTION("index out of range");
        return NONE;
    }
    else {
        return TUPLE_GET_ITEM(tp, index);
    }
}

int tuple_setitem(AiTupleObject *tp, ssize_t index, AiObject *newitem) {
    AiObject *olditem;
    if (index < 0 || index >= TUPLE_SIZE(tp)) {
        RUNTIME_EXCEPTION("index out of range");
        return -1;
    }
    else {
        olditem = TUPLE_GET_ITEM(tp, index);
        XDEC_REFCNT(olditem);
        TUPLE_SET_ITEM(tp, index, newitem);
        return 0;
    }
}

AiObject *tuple_slice(AiTupleObject *tp, ssize_t start, ssize_t end) {
    AiTupleObject *np;
    MAKE_INDEX_IN_RANGE(start, TUPLE_SIZE(tp));
    MAKE_INDEX_IN_RANGE(end, TUPLE_SIZE(tp));
    if (start < TUPLE_SIZE(tp) && start < end) {
        if (end > TUPLE_SIZE(tp)) {
            end = TUPLE_SIZE(tp);
        }
        if (start == 0 && end == TUPLE_SIZE(tp)) {
            INC_REFCNT(tp);
            return (AiObject *)tp;
        }
        else {
            np = (AiTupleObject *)tuple_new(end - start);
            for (ssize_t i = 0; i < end - start; ++i) {
                np->ob_item[i] = tp->ob_item[start + i];
                XINC_REFCNT(np->ob_item[i]);
            }
            return (AiObject *)np;
        }
    }
    else {
        RUNTIME_EXCEPTION("invalid range sliced");
        return NONE;
    }
}

int tuple_resize(AiTupleObject *tp, ssize_t newsize) {
    FATAL_ERROR("not implemented yet");
    return 0;
}

AiObject *tuple_pack(ssize_t argc, ...) {
    AiObject *r;
    va_list vargs;

    va_start(vargs, argc);
    r = tuple_new(argc);
    for (ssize_t i = 0; i < argc; ++i) {
        TUPLE_GET_ITEM(r, i) = va_arg(vargs, AiObject *);
        INC_REFCNT(TUPLE_GET_ITEM(r, i));
    }
    va_end(vargs);

    return r;
}
