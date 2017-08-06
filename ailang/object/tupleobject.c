#include "../ailang.h"

static AiObject *tuple_slice(AiTupleObject *tp, ssize_t start, ssize_t end);
static void tuple_dealloc(AiTupleObject *tp);
static void tuple_print(AiTupleObject *tp, FILE *stream);
static long tuple_hash(AiTupleObject *v);
static AiObject *tuple_str(AiTupleObject *tp);
static int tuple_contains(AiTupleObject *tp, AiObject *item);
static AiObject *tuple_iter(AiObject *seq);

static AiTupleObject *free_tuples[NUMBER_FREE_TUPLES_MAX];
static int number_free_tuples[NUMBER_FREE_TUPLES_MAX];

static AiMethodDef tuple_methods[] = {
    { NULL }
};

static AiSequenceMethods tuple_as_sequence = {
    (lengthfunc)tuple_size,
    0,
    (ssizeargfunc)AiTuple_GetItem,
    (sqsetitemfunc)AiTuple_SetItem,
    (ssize2argfunc)tuple_slice,
    (enquiry2)tuple_contains,
};

AiTypeObject AiType_Tuple = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "tuple",                            /* tp_name */
    sizeof(AiTupleObject),              /* tp_basicsize */
    sizeof(AiObject *),                 /* tp_itemsize */
    (destructor)tuple_dealloc,          /* tp_dealloc */
    (printfunc)tuple_print,             /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    &tuple_as_sequence,                 /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    (hashfunc)tuple_hash,               /* tp_hash */
    0,                                  /* tp_call */
    (unaryfunc)tuple_str,               /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//AiObject_Generic_Getattr,             /* tp_getattro */
    0,                                  /* tp_setattro */

    SUBCLASS_TUPLE | BASE_TYPE,         /* tp_flags */

    tuple_iter,                         /* tp_iter */
    0,                                  /* tp_iternext */

    tuple_methods,                      /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,//AiTuple_New,                          /* tp_new */
    AiObject_GC_Del,                    /* tp_free */
};

AiObject *AiTuple_New(ssize_t size) {
    AiTupleObject *tp;

    if (size == 0 && (tp = free_tuples[0])) {
        INC_REFCNT(tp);
        return (AiObject *)tp;
    }
    else if (size < NUMBER_FREE_TUPLES_MAX && (tp = free_tuples[size])) {
        free_tuples[size] = (AiTupleObject *)tp->ob_type;
        --number_free_tuples[size];
        AiVarObject_INIT(tp, &AiType_Tuple, size);
    }
    else {
        tp = AiVarObject_NEW(AiTupleObject, &AiType_Tuple, max(size - 1, 0));
    }
    AiMem_Set(tp->ob_item, 0, size * sizeof(AiObject *));
    if (size == 0) {
        free_tuples[0] = tp;
        ++number_free_tuples[0];
        INC_REFCNT(tp);
    }
    return (AiObject *)tp;
}

AiObject *AiTuple_GetItem(AiTupleObject *tp, ssize_t index) {
    if (index < 0 || index >= TUPLE_SIZE(tp)) {
        RUNTIME_EXCEPTION("index out of range");
        return NULL;
    }
    else {
        return TUPLE_GETITEM(tp, index);
    }
}

int AiTuple_SetItem(AiTupleObject *tp, ssize_t index, AiObject *newitem) {
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
            np = (AiTupleObject *)AiTuple_New(end - start);
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

AiObject *AiTuple_Pack(ssize_t argc, ...) {
    AiObject *r;
    va_list vargs;

    va_start(vargs, argc);
    r = AiTuple_New(argc);
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
    if (CHECK_EXACT_TYPE_TUPLE(tp) && size < NUMBER_FREE_TUPLES_MAX) {
        ++number_free_tuples[size];
        tp->ob_type = (AiTypeObject *)free_tuples[size];
        free_tuples[size] = tp;
    }
    else {
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

long tuple_hash(AiTupleObject *v) {
    long x, y;
    ssize_t len = TUPLE_SIZE(v);
    AiObject **p;
    long mult = 1000003L;
    x = 0x345678L;
    p = v->ob_item;
    while (--len >= 0) {
        y = AiObject_Generic_Hash(*p++);
        if (y == -1) {
            return -1;
        }
        x = (x ^ y) * mult;
        mult += (long)(82520L + len + len);
    }
    x += 97531L;
    if (x == -1) {
        x = -2;
    }
    return x;
}

AiObject *tuple_str(AiTupleObject *tp) {
    AiListObject *strlist;
    AiStringObject *str;
    AiStringObject *item;
    AiStringObject *split;
    ssize_t size;

    if (TUPLE_SIZE(tp) == 0) {
        return AiString_From_String("()");
    }
    else if (TUPLE_SIZE(tp) == 1) {
        item = (AiStringObject *)OB_TO_STRING(tp->ob_item[0]);
        str = (AiStringObject *)AiString_From_StringAndSize(NULL, STRING_LEN(item) + 2);
        STRING_AS_CSTRING(str)[0] = '(';
        AiMem_Copy(&STRING_AS_CSTRING(str)[1], STRING_AS_CSTRING(item), STRING_LEN(item));
        STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = ')';
        DEC_REFCNT(item);
        return (AiObject *)str;
    }

    strlist = (AiListObject *)AiList_New(TUPLE_SIZE(tp));

    item = (AiStringObject *)OB_TO_STRING(tp->ob_item[0]);
    size = STRING_LEN(item);
    strlist->ob_item[0] = AiString_From_StringAndSize(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[0])[0] = '(';
    AiMem_Copy(&STRING_AS_CSTRING(strlist->ob_item[0])[1], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    item = (AiStringObject *)OB_TO_STRING(tp->ob_item[TUPLE_SIZE(tp) - 1]);
    size = STRING_LEN(item);
    strlist->ob_item[TUPLE_SIZE(tp) - 1] = AiString_From_StringAndSize(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[TUPLE_SIZE(tp) - 1])[size] = ')';
    AiMem_Copy(&STRING_AS_CSTRING(strlist->ob_item[TUPLE_SIZE(tp) - 1])[0], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    for (ssize_t i = 1; i < TUPLE_SIZE(tp) - 1; ++i) {
        strlist->ob_item[i] = OB_TO_STRING(tp->ob_item[i]);
    }
    split = (AiStringObject *)AiString_From_String(", ");
    str = (AiStringObject *)string_join(split, (AiObject *)strlist);

    split->ob_type->tp_dealloc((AiObject *)split);
    strlist->ob_type->tp_dealloc((AiObject *)strlist);

    return (AiObject *)str;
}

int tuple_contains(AiTupleObject *tp, AiObject *item) {
    for (ssize_t i = 0; i < TUPLE_SIZE(tp); ++i) {
        if (AiObject_Rich_Compare(tp->ob_item[i], item, CMP_EQ) > 0) {
            return 1;
        }
    }
    return 0;
}

AiObject *tuple_iter(AiObject *seq) {
    AiSeqiterObject *it;

    it = AiObject_NEW(AiSeqiterObject, &AiType_Seqiter);
    it->it_index = 0;
    INC_REFCNT(seq);

    // TODO
    it->it_seq = (AiListObject *)seq;
    return (AiObject *)it;
}
