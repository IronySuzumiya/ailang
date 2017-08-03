#include "../ailang.h"

static void tuple_dealloc(AiTupleObject *tp);
static void tuple_print(AiTupleObject *tp, FILE *stream);
static AiObject *tuple_tostring(AiTupleObject *tp);
static int tuple_contains(AiTupleObject *tp, AiObject *item);
static void tuple_free(void *p);

static AiTupleObject *free_tuples[NUMBER_FREE_TUPLES_MAX];
static int number_free_tuples[NUMBER_FREE_TUPLES_MAX];

static sequencemethods tuple_as_sequence = {
    (lengthfunc)tuple_size,
    0,
    (ssizeargfunc)tuple_getitem,
    (sqsetitemfunc)tuple_setitem,
    (ssize2argfunc)tuple_slice,
    (enquiry2)tuple_contains,
};

AiTypeObject type_tupleobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "tuple",
    (destructor)tuple_dealloc,
    (printfunc)tuple_print,
    0,

    0,
    &tuple_as_sequence,
    0,

    (hashfunc)pointer_hash,
    (unaryfunc)tuple_tostring,
    (freefunc)tuple_free,
};

AiObject *tuple_new(ssize_t size) {
    AiTupleObject *tp;

    if (size == 0 && (tp = free_tuples[0])) {
        INC_REFCNT(tp);
        return (AiObject *)tp;
    }
    else if (size < NUMBER_FREE_TUPLES_MAX && (tp = free_tuples[size])) {
        free_tuples[size] = (AiTupleObject *)tp->ob_type;
        --number_free_tuples[size];
        INIT_OBJECT_VAR(tp, &type_tupleobject, size);
    }
    else {
        tp = AiObject_GC_NEW(AiTupleObject);
        INIT_OBJECT_VAR(tp, &type_tupleobject, size);
        tp->ob_item = AiMEM_ALLOC(size * sizeof(AiObject *));
    }
    for (ssize_t i = 0; i < size; ++i) {
        tp->ob_item[i] = 0;
    }

    if (size == 0) {
        free_tuples[0] = tp;
        ++number_free_tuples[0];
        INC_REFCNT(tp);
    }
    return (AiObject *)tp;
}

AiObject *tuple_getitem(AiTupleObject *tp, ssize_t index) {
    if (index < 0 || index >= TUPLE_SIZE(tp)) {
        RUNTIME_EXCEPTION("index out of range");
        return NULL;
    }
    else {
        return TUPLE_GETITEM(tp, index);
    }
}

int tuple_setitem(AiTupleObject *tp, ssize_t index, AiObject *newitem) {
    AiObject *olditem;
    if (index < 0 || index >= TUPLE_SIZE(tp)) {
        RUNTIME_EXCEPTION("index out of range");
        return -1;
    }
    else {
        olditem = TUPLE_GETITEM(tp, index);
        XDEC_REFCNT(olditem);
        TUPLE_SETITEM(tp, index, newitem);
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
        return NULL;
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
        TUPLE_GETITEM(r, i) = va_arg(vargs, AiObject *);
        INC_REFCNT(TUPLE_GETITEM(r, i));
    }
    va_end(vargs);

    return r;
}

ssize_t tuple_size(AiTupleObject *tp) {
    return CHECK_TYPE_TUPLE(tp) ? TUPLE_SIZE(tp) : tp->ob_type->tp_as_sequence->sq_length((AiObject *)tp);
}

void tuple_dealloc(AiTupleObject *tp) {
    ssize_t size = TUPLE_SIZE(tp);

    if (tp->ob_item) {
        for (ssize_t i = 0; i < size; ++i) {
            XDEC_REFCNT(tp->ob_item[i]);
        }
    }
    if (CHECK_TYPE_TUPLE(tp) && size < NUMBER_FREE_TUPLES_MAX) {
        ++number_free_tuples[size];
        tp->ob_type = (AiTypeObject *)free_tuples[size];
        free_tuples[size] = tp;
    }
    else {
        AiMEM_FREE(tp->ob_item);
        OB_FREE(tp);
    }
}

void tuple_print(AiTupleObject *tp, FILE *stream) {
    fputs("(", stream);
    if (tp->ob_item && TUPLE_SIZE(tp) > 0) {
        for (ssize_t i = 0; i < TUPLE_SIZE(tp) - 1; ++i) {
            if (!tp->ob_item[i])
                continue;
            OB_PRINT(tp->ob_item[i], stream);
            fputs(", ", stream);
        }
        OB_PRINT(tp->ob_item[TUPLE_SIZE(tp) - 1], stream);
    }
    fputs(")", stream);
}

AiObject *tuple_tostring(AiTupleObject *tp) {
    AiListObject *strlist;
    AiStringObject *str;
    AiStringObject *item;
    AiStringObject *split;
    ssize_t size;

    if (TUPLE_SIZE(tp) == 0) {
        return string_from_cstring("()");
    }
    else if (TUPLE_SIZE(tp) == 1) {
        item = (AiStringObject *)OB_TO_STRING(tp->ob_item[0]);
        str = (AiStringObject *)string_from_cstring_with_size(NULL, STRING_LEN(item) + 2);
        STRING_AS_CSTRING(str)[0] = '(';
        AiMEM_COPY(&STRING_AS_CSTRING(str)[1], STRING_AS_CSTRING(item), STRING_LEN(item));
        STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = ')';
        DEC_REFCNT(item);
        return (AiObject *)str;
    }

    strlist = (AiListObject *)list_new(TUPLE_SIZE(tp));

    item = (AiStringObject *)OB_TO_STRING(tp->ob_item[0]);
    size = STRING_LEN(item);
    strlist->ob_item[0] = string_from_cstring_with_size(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[0])[0] = '(';
    AiMEM_COPY(&STRING_AS_CSTRING(strlist->ob_item[0])[1], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    item = (AiStringObject *)OB_TO_STRING(tp->ob_item[TUPLE_SIZE(tp) - 1]);
    size = STRING_LEN(item);
    strlist->ob_item[TUPLE_SIZE(tp) - 1] = string_from_cstring_with_size(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[TUPLE_SIZE(tp) - 1])[size] = ')';
    AiMEM_COPY(&STRING_AS_CSTRING(strlist->ob_item[TUPLE_SIZE(tp) - 1])[0], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    for (ssize_t i = 1; i < TUPLE_SIZE(tp) - 1; ++i) {
        strlist->ob_item[i] = OB_TO_STRING(tp->ob_item[i]);
    }
    split = (AiStringObject *)string_from_cstring(", ");
    str = (AiStringObject *)string_join(split, (AiObject *)strlist);

    split->ob_type->tp_dealloc((AiObject *)split);
    strlist->ob_type->tp_dealloc((AiObject *)strlist);

    return (AiObject *)str;
}

int tuple_contains(AiTupleObject *tp, AiObject *item) {
    for (ssize_t i = 0; i < TUPLE_SIZE(tp); ++i) {
        if (object_rich_compare(tp->ob_item[i], item, CMP_EQ) > 0) {
            return 1;
        }
    }
    return 0;
}

void tuple_free(void *p) {
    AiMEM_FREE(p);
}
