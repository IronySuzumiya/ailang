#include "../ailang.h"

AiIntBlock *block_list;
AiIntObject *free_list;

static void int_dealloc(AiIntObject *ob);
static void int_print(AiIntObject *ob, FILE *stream);
static int int_compare(AiIntObject *lhs, AiIntObject *rhs);
static long int_hash(AiIntObject *ob);
static void int_to_cstring(AiIntObject *ob, char *buffer, int radix);
static AiObject *int_to_string_with_radix(AiIntObject *ob, int radix);
static AiObject *int_to_string(AiIntObject *ob);
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
static AiObject *int_xor(AiIntObject *lhs, AiIntObject *rhs);
static AiIntObject *fill_free_list(void);

static numbermethods int_as_number = {
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
    (unaryfunc)int_not,
    (binaryfunc)int_xor,

    0,
    0,
    0,
};

AiTypeObject type_intobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "int",                          /* tp_name */
    (destructor)int_dealloc,        /* tp_dealloc */
    (printfunc)int_print,           /* tp_print */
    (cmpfunc)int_compare,           /* tp_compare */
    &int_as_number,                 /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    (hashfunc)int_hash,             /* tp_hash */
    (unaryfunc)int_to_string,       /* tp_to_string */
    0,                              /* tp_free */
};

AiIntObject *small_intobject_buf[SMALL_INTOBJECT_BUF_SIZE];

AiObject *int_from_long(long ival) {
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
        INIT_OBJECT(v, &type_intobject);
        v->ob_ival = ival;
        return (AiObject *)v;
    }
}

void int_init() {
    AiIntObject *v;
    for (long i = -NUM_NEG_SMALL_INTOBJECT; i < NUM_POS_SMALL_INTOBJECT; ++i) {
        if (!free_list && !(free_list = fill_free_list())) {
            FATAL_ERROR("bad free list allocating");
        }
        v = free_list;
        free_list = (AiIntObject *)v->ob_type;
        INIT_OBJECT(v, &type_intobject);
        v->ob_ival = i;
        small_intobject_buf[SMALL_INTOBJECT_INDEX(i)] = v;
    }
}

void int_dealloc(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        ob->ob_type = (AiTypeObject *)free_list;
        free_list = ob;
    }
    else {
        OB_FREE(ob);
    }
}

void int_print(AiIntObject *ob, FILE *stream) {
    if (CHECK_TYPE_INT(ob)) {
        fprintf(stream, "%d\n", ob->ob_ival);
    }
    else {
        ob->ob_type->tp_print((AiObject *)ob, stream);
    }
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

AiObject *int_to_string_with_radix(AiIntObject *ob, int radix) {
    if (CHECK_TYPE_INT(ob)) {
        char buffer[INT_TO_CSTRING_BUFFER_SIZE];
        int_to_cstring(ob, buffer, radix);
        return string_from_cstring(buffer);
    }
    else {
        return OB_TO_STRING(ob);
    }
}

AiObject *int_to_string(AiIntObject *ob) {
    return int_to_string_with_radix(ob, 10);
}

AiObject *int_add(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, +);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_ADD(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_sub(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, -);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_SUB(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_mul(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, *);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_MUL(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_div(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, /);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_DIV(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_mod(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, %);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_MOD(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_pow(AiIntObject *lhs, AiIntObject *rhs) {
    if (CHECK_TYPE_INT(lhs) && CHECK_TYPE_INT(rhs)) {
        return int_from_long((long)pow(lhs->ob_ival, rhs->ob_ival));
    }
    else {
        UNSUPPORTED_POW(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_pos(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return int_from_long(ob->ob_ival);
    }
    else {
        UNSUPPORTED_POS(OB_TYPENAME(ob));
        return NONE;
    }
}

AiObject *int_neg(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return int_from_long(-ob->ob_ival);
    }
    else {
        UNSUPPORTED_NEG(OB_TYPENAME(ob));
        return NONE;
    }
}

AiObject *int_abs(AiIntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        return int_from_long(ob->ob_ival >= 0 ? ob->ob_ival : -ob->ob_ival);
    }
    else {
        UNSUPPORTED_ABS(OB_TYPENAME(ob));
        return NONE;
    }
}

int int_nonzero(AiIntObject *ob) {
    return CHECK_TYPE_INT(ob) ?
        (ob->ob_ival != 0 ? 1 : 0) : ob->ob_type->tp_as_number->nb_nonzero((AiObject *)ob);
}

AiObject *int_shl(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, <<);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_SHL(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_shr(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, >>);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_SHR(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_and(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, &);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_AND(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_or(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, |);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_OR(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *int_not(AiIntObject *ob) {
    AiObject *r = INT_UNARY_WITH_CHECK(ob, ~);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_NOT(OB_TYPENAME(ob));
        return NONE;
    }
}

AiObject *int_xor(AiIntObject *lhs, AiIntObject *rhs) {
    AiObject *r = INT_BINARY_WITH_CHECK(lhs, rhs, ^);
    if (r) {
        return r;
    }
    else {
        UNSUPPORTED_XOR(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiIntObject *fill_free_list() {
    AiIntObject *p, *q;
    p = AiMEM_ALLOC(sizeof(AiIntBlock));
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
