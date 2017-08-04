#include "../ailang.h"

static void int_dealloc(AiIntObject *ob);
static void int_print(AiIntObject *ob, FILE *stream);
static int int_compare(AiIntObject *lhs, AiIntObject *rhs);
static long int_hash(AiIntObject *ob);
static void int_to_cstring(AiIntObject *ob, char *buffer, int radix);
static AiObject *int_str_with_radix(AiIntObject *ob, int radix);
static AiObject *int_str(AiIntObject *ob);
static AiObject *int_add(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_sub(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_mul(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_div(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_mod(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_pow(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_pos(AiIntObject *ob);
static AiObject *int_neg(AiIntObject *ob);
static AiObject *int_abs(AiIntObject *ob);
static int int_nonzero(AiIntObject *ob);
static AiObject *int_shl(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_shr(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_and(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_or(AiIntObject *lhs, AiIntObject *rhs);
static AiObject *int_not(AiIntObject *ob);
static AiObject *int_invert(AiIntObject *ob);
static AiObject *int_xor(AiIntObject *lhs, AiIntObject *rhs);
static AiIntObject *fill_free_list(void);

static AiNumberMethods int_as_number = {
    (binaryfunc)int_add,
    (binaryfunc)int_sub,
    (binaryfunc)int_mul,
    (binaryfunc)int_div,
    (binaryfunc)int_mod,
    (binaryfunc)int_pow,

    (unaryfunc)int_pos,
    (unaryfunc)int_neg,
    (unaryfunc)int_abs,

    (enquiry)int_nonzero,

    (binaryfunc)int_shl,
    (binaryfunc)int_shr,
    (binaryfunc)int_and,
    (binaryfunc)int_or,
    (binaryfunc)int_xor,
    (unaryfunc)int_not,
    (unaryfunc)int_invert,

    0,
    0,
};

AiTypeObject type_intobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "int",                          /* tp_name */
    sizeof(AiIntObject),            /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)int_dealloc,        /* tp_dealloc */
    (printfunc)int_print,           /* tp_print */
    (cmpfunc)int_compare,           /* tp_compare */

    &int_as_number,                 /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */

    (hashfunc)int_hash,             /* tp_hash */
    0,                              /* tp_call */
    (unaryfunc)int_str,             /* tp_str */

    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,//object_generic_getattr,         /* tp_getattro */
    0,                              /* tp_setattro */

    SUBCLASS_INT | BASE_TYPE,       /* tp_flags */

    0,                              /* tp_iter */
    0,                              /* tp_iternext */

    0,//int_methods,                    /* tp_methods */
    0,                              /* tp_members */
    0,//int_getset,                     /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    0,//int_new,                        /* tp_new */
};

AiIntBlock *block_list;
AiIntObject *free_list;

AiIntObject *small_intobject_buf[SMALL_INTOBJECT_BUF_SIZE];

AiObject *int_from_clong(long ival) {
    AiIntObject *v = 0;
    if (ival >= -NUM_NEG_SMALL_INTOBJECT && ival < NUM_POS_SMALL_INTOBJECT) {
        v = small_intobject_buf[SMALL_INTOBJECT_INDEX(ival)];
        INC_REFCNT(v);
        return (AiObject *)v;
    }
    else {
        if (!free_list && !(free_list = fill_free_list())) {
            FATAL_ERROR("bad free list allocating");
            return NULL;
        }
        v = free_list;
        free_list = (AiIntObject *)v->ob_type;
        INIT_AiObject(v, &type_intobject);
        v->ob_ival = ival;
        return (AiObject *)v;
    }
}

int int_init() {
    AiIntObject *v;
    for (long i = -NUM_NEG_SMALL_INTOBJECT; i < NUM_POS_SMALL_INTOBJECT; ++i) {
        if (!free_list && !(free_list = fill_free_list())) {
            FATAL_ERROR("bad free list allocating");
            return -1;
        }
        v = free_list;
        free_list = (AiIntObject *)v->ob_type;
        INIT_AiObject(v, &type_intobject);
        v->ob_ival = i;
        small_intobject_buf[SMALL_INTOBJECT_INDEX(i)] = v;
    }
    return 0;
}

int int_clear_blocks(void) {
    AiIntBlock *s = block_list;
    AiIntBlock *n = s;

    while (s) {
        n = s->next;
        AiMEM_FREE(s);
        s = n;
    }
    return 0;
}

void int_dealloc(AiIntObject *ob) {
    if (CHECK_EXACT_TYPE_INT(ob)) {
        OB_TYPE(ob) = (AiTypeObject *)free_list;
        free_list = ob;
    }
    else {
        OB_FREE(ob);
    }
}

void int_print(AiIntObject *ob, FILE *stream) {
    fprintf(stream, "%ld", ob->ob_ival);
}

int int_compare(AiIntObject *lhs, AiIntObject *rhs) {
    return lhs->ob_ival < rhs->ob_ival ? -1 : lhs->ob_ival > rhs->ob_ival ? 1 : 0;
}

long int_hash(AiIntObject *ob) {
    long x = ob->ob_ival;
    return x == -1 ? -2 : x;
}

void int_to_cstring(AiIntObject *ob, char *buffer, int radix) {
    _ltoa_s(ob->ob_ival, buffer, INT_TO_CSTRING_BUFFER_SIZE, radix);
}

AiObject *int_str_with_radix(AiIntObject *ob, int radix) {
    char buffer[INT_TO_CSTRING_BUFFER_SIZE];
    int_to_cstring(ob, buffer, radix);
    return string_from_cstring(buffer);
}

AiObject *int_str(AiIntObject *ob) {
    return int_str_with_radix(ob, 10);
}

AiObject *int_add(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, +);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_ADD(lhs, rhs);
        return NULL;
    }
}

AiObject *int_sub(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, -);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_SUB(lhs, rhs);
        return NULL;
    }
}

AiObject *int_mul(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, *);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_MUL(lhs, rhs);
        return NULL;
    }
}

AiObject *int_div(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, /);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_DIV(lhs, rhs);
        return NULL;
    }
}

AiObject *int_mod(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, %);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_MOD(lhs, rhs);
        return NULL;
    }
}

AiObject *int_pow(AiIntObject *lhs, AiIntObject *rhs) {
    if (CHECK_TYPE_INT(lhs) && CHECK_TYPE_INT(rhs)) {
        return int_from_clong((long)pow(lhs->ob_ival, rhs->ob_ival));
    }
    else {
        UNSUPPORTED_POW(lhs, rhs);
        return NULL;
    }
}

AiObject *int_pos(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return int_from_clong(ob->ob_ival);
    }
    else {
        UNSUPPORTED_POS(ob);
        return NULL;
    }
}

AiObject *int_neg(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return int_from_clong(-ob->ob_ival);
    }
    else {
        UNSUPPORTED_NEG(ob);
        return NULL;
    }
}

AiObject *int_abs(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return int_from_clong(ob->ob_ival >= 0 ? ob->ob_ival : -ob->ob_ival);
    }
    else {
        UNSUPPORTED_ABS(ob);
        return NULL;
    }
}

int int_nonzero(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return ob->ob_ival ? 1 : 0;
    }
    else {
        UNSUPPORTED_NONZERO(ob);
        return -1;
    }
}

AiObject *int_shl(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, <<);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_SHL(lhs, rhs);
        return NULL;
    }
}

AiObject *int_shr(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, >>);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_SHR(lhs, rhs);
        return NULL;
    }
}

AiObject *int_and(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, &);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_AND(lhs, rhs);
        return NULL;
    }
}

AiObject *int_or(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, |);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_OR(lhs, rhs);
        return NULL;
    }
}

AiObject *int_xor(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, ^);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_XOR(lhs, rhs);
        return NULL;
    }
}

AiObject *int_not(AiIntObject *ob) {
    AiObject *r = INT_UNARY_WITH_CHECK(ob, !);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_NOT(ob);
        return NULL;
    }
}

AiObject *int_invert(AiIntObject *ob) {
    AiObject *r = INT_UNARY_WITH_CHECK(ob, ~);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_NOT(ob);
        return NULL;
    }
}

AiIntObject *fill_free_list() {
    AiIntObject *p, *q;
    p = AiObject_GC_NEW(AiIntBlock);
    if (!p) {
        return NULL;
    }
    ((AiIntBlock *)p)->next = block_list;
    block_list = (AiIntBlock *)p;
    p = ((AiIntBlock *)p)->block;
    q = p + NUMBER_INTOBJECT_PER_BLOCK;
    while (--q > p) {
        q->ob_type = (AiTypeObject *)(q - 1);
    }
    q->ob_type = NULL;
    return p + NUMBER_INTOBJECT_PER_BLOCK - 1;
}
