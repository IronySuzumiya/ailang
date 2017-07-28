#include "../ailang.h"

IntBlock *block_list;
IntObject *free_list;

static void int_dealloc(IntObject *ob);
static void int_print(IntObject *ob, FILE *stream);
static int int_compare(IntObject *lhs, IntObject *rhs);
static long int_hash(IntObject *ob);
static Object *int_to_string_with_radix(IntObject *ob, int radix);
static Object *int_to_string(IntObject *ob);
static Object *int_add(IntObject *lhs, IntObject *rhs);
static Object *int_sub(IntObject *lhs, IntObject *rhs);
static Object *int_mul(IntObject *lhs, IntObject *rhs);
static Object *int_div(IntObject *lhs, IntObject *rhs);
static Object *int_mod(IntObject *lhs, IntObject *rhs);
static Object *int_pow(IntObject *lhs, IntObject *rhs);
static Object *int_pos(IntObject *ob);
static Object *int_neg(IntObject *ob);
static Object *int_abs(IntObject *ob);
static int int_nonzero(IntObject *ob);
static Object *int_shl(IntObject *lhs, IntObject *rhs);
static Object *int_shr(IntObject *lhs, IntObject *rhs);
static Object *int_and(IntObject *lhs, IntObject *rhs);
static Object *int_or(IntObject *lhs, IntObject *rhs);
static Object *int_not(IntObject *ob);
static IntObject *fill_free_list(void);

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

    0,
    0,
    0,
};

TypeObject type_intobject = {
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

IntObject *small_intobject_buf[SMALL_INTOBJECT_BUF_SIZE];

Object *int_fromlong(long ival) {
    IntObject *v = 0;
    if (ival >= -NUM_NEG_SMALL_INTOBJECT && ival < NUM_POS_SMALL_INTOBJECT) {
        v = small_intobject_buf[SMALL_INTOBJECT_INDEX(ival)];
        INC_REFCNT(v);
        return (Object *)v;
    }
    else {
        if (!free_list && !(free_list = fill_free_list())) {
            FATAL_ERROR("bad free list allocating");
            return NULL;
        }
        v = free_list;
        free_list = (IntObject *)v->ob_type;
        INIT_OBJECT(v, &type_intobject);
        v->ob_ival = ival;
        return (Object *)v;
    }
}

void int_init() {
    IntObject *v;
    for (long i = -NUM_NEG_SMALL_INTOBJECT; i < NUM_POS_SMALL_INTOBJECT; ++i) {
        if (!free_list && !(free_list = fill_free_list())) {
            FATAL_ERROR("bad free list allocating");
        }
        v = free_list;
        free_list = (IntObject *)v->ob_type;
        INIT_OBJECT(v, &type_intobject);
        v->ob_ival = i;
        small_intobject_buf[SMALL_INTOBJECT_INDEX(i)] = v;
    }
}

void int_dealloc(IntObject *ob) {
    if (CHECK_TYPE_INT(ob)) {
        ob->ob_type = (TypeObject *)free_list;
        free_list = ob;
    }
    else {
        ob->ob_type->tp_free(ob);
    }
}

void int_print(IntObject *ob, FILE *stream) {
    if (CHECK_TYPE_INT(ob)) {
        fprintf(stream, "<type 'int'> <value %d> <addr %p>\n", ob->ob_ival, ob);
    }
    else {
        ob->ob_type->tp_print((Object *)ob, stream);
    }
}

int int_compare(IntObject *lhs, IntObject *rhs) {
    return lhs->ob_ival < rhs->ob_ival ? -1 : lhs->ob_ival > rhs->ob_ival ? 1 : 0;
}

long int_hash(IntObject *ob) {
    long x = ob->ob_ival;
    return x == -1 ? -2 : x;
}

Object *int_to_string_with_radix(IntObject *ob, int radix) {
    if (CHECK_TYPE_INT(ob)) {
        char buffer[33];
        _ltoa_s(ob->ob_ival, buffer, 33, radix);
        return string_fromcstring(buffer);
    }
    else {
        return ob->ob_type->tp_to_string((Object *)ob);
    }
}

Object *int_to_string(IntObject *ob) {
    return int_to_string_with_radix(ob, 10);
}

Object *int_add(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, +);
}

Object *int_sub(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, -);
}

Object *int_mul(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, *);
}

Object *int_div(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, /);
}

Object *int_mod(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, %);
}

Object *int_pow(IntObject *lhs, IntObject *rhs) {
    return CHECK_TYPE_INT(lhs) && CHECK_TYPE_INT(rhs) ?
        int_fromlong((long)pow(lhs->ob_ival, rhs->ob_ival)) : NULL;
}

Object *int_pos(IntObject *ob) {
    return CHECK_TYPE_INT(ob) ?
        int_fromlong(ob->ob_ival) : NULL;
}

Object *int_neg(IntObject *ob) {
    return CHECK_TYPE_INT(ob) ?
        int_fromlong(-ob->ob_ival) : NULL;
}

Object *int_abs(IntObject *ob) {
    return CHECK_TYPE_INT(ob) ?
        int_fromlong(ob->ob_ival >= 0 ? ob->ob_ival : -ob->ob_ival) : NULL;
}

int int_nonzero(IntObject *ob) {
    return CHECK_TYPE_INT(ob) ?
        (ob->ob_ival != 0 ? 1 : 0) : -1;
}

Object *int_shl(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, <<);
}

Object *int_shr(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, >>);
}

Object *int_and(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, &);
}

Object *int_or(IntObject *lhs, IntObject *rhs) {
    return INT_BINARY_WITH_CHECK(lhs, rhs, |);
}

Object *int_not(IntObject *ob) {
    return INT_UNARY_WITH_CHECK(ob, ~);
}

IntObject *fill_free_list() {
    IntObject *p, *q;
    p = gc_malloc(sizeof(IntBlock));
    ((IntBlock *)p)->next = block_list;
    block_list = (IntBlock *)p;
    p = ((IntBlock *)p)->block;
    q = p + NUMBER_INTOBJECT_PER_BLOCK;
    while (--q > p) {
        q->ob_type = (TypeObject *)(q - 1);
    }
    q->ob_type = NULL;
    return p + NUMBER_INTOBJECT_PER_BLOCK - 1;
}
