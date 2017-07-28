#include "../ailang.h"

static StringObject *characters[UCHAR_MAX + 1];
static Object *interned;

static Object *_string_fromcstring_withsize(const char *sval, ssize_t size);
static void string_dealloc(StringObject *a);
static void string_print(StringObject *a, FILE *stream);
static int string_compare(StringObject *lhs, StringObject *rhs);
static long string_hash(StringObject *a);
static Object *string_to_string(StringObject *a);
static ssize_t string_length(StringObject *a);
static Object *string_concat(StringObject *lhs, StringObject *rhs);
static Object *string_getitem(StringObject *a, ssize_t index);
static Object *string_slice(StringObject *a, ssize_t start, ssize_t end);
static int string_contains(StringObject *a, StringObject *sub);

// string object is immutable
static sequencemethods string_as_sequence = {
    (lengthfunc)string_length,
    (binaryfunc)string_concat,
    (ssizeargfunc)string_getitem,
    0,
    (ssize2argfunc)string_slice,
    (enquiry2)string_contains,
};

TypeObject type_stringobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "string",                                   /* tp_name */
    (destructor)string_dealloc,                 /* tp_dealloc */
    (printfunc)string_print,                    /* tp_print */
    (cmpfunc)string_compare,                    /* tp_compare */
    0,                                          /* tp_as_number */
    &string_as_sequence,                        /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    (hashfunc)string_hash,                      /* tp_hash */
    (unaryfunc)string_to_string,                /* tp_to_string */
    (freefunc)gc_free,                          /* tp_free */
};

StringObject *nullstring;

Object *string_fromcstring(const char *sval) {
    return _string_fromcstring_withsize(sval, strlen(sval));
}

Object *string_fromcstring_withsize(const char *sval, ssize_t size) {
    if (size < 0) {
        size = 0;
    }
    else {
        ssize_t maxsize = strlen(sval);
        size = min(size, maxsize);
    }
    return _string_fromcstring_withsize(sval, size);
}

void string_intern(StringObject **a) {
    Object *s = (Object *)(*a);
    Object *t;

    if (!CHECK_TYPE_STRING(s) || CHECK_STRING_INTERNED(s)) {
        return;
    }

    if (!interned) {
        interned = dict_new();
    }

    if (t = dict_getitem(interned, s)) {
        INC_REFCNT(t);
        DEC_REFCNT(*a);
        *a = (StringObject *)t;
    }
    else {
        dict_setitem(interned, s, s);
        s->ob_refcnt -= 2;
        CHECK_STRING_INTERNED(s) = SSTATE_INTERNED_MORTAL;
    }
}

void string_intern_immortal(StringObject **a) {
    string_intern(a);
    if (CHECK_STRING_INTERNED(*a) != SSTATE_INTERNED_IMMORTAL) {
        CHECK_STRING_INTERNED(*a) = SSTATE_INTERNED_IMMORTAL;
        INC_REFCNT(*a);
    }
}

Object *_string_fromcstring_withsize(const char *sval, ssize_t size) {
    StringObject *a;

    if (size == 0 && (a = nullstring)) {
        return (Object *)nullstring;
    }
    else if (size == 1 && (a = characters[*sval & UCHAR_MAX])) {
        return (Object *)a;
    }
    else {
        a = gc_malloc(sizeof(StringObject) + size);
        INIT_OBJECT_VAR(a, &type_stringobject, size);
        a->ob_shash = -1;
        a->ob_sstate = SSTATE_NOT_INTERNED;
        memcpy(a->ob_sval, sval, (size_t)size + 1);
        if (size == 0) {
            string_intern(&a);
            nullstring = a;
            INC_REFCNT(a);
        }
        else if (size == 1) {
            string_intern(&a);
            characters[*sval & UCHAR_MAX] = a;
            INC_REFCNT(a);
        }
        return (Object *)a;
    }
}

void string_dealloc(StringObject *a) {
    if (CHECK_TYPE_STRING(a)) {
        switch (CHECK_STRING_INTERNED(a))
        {
        case SSTATE_NOT_INTERNED:
            break;
        case SSTATE_INTERNED_MORTAL:
            a->ob_refcnt = 3;
            if (dict_delitem(interned, (Object *)a)) {
                FATAL_ERROR("deletion of interned string failed");
            }
            break;
        case SSTATE_INTERNED_IMMORTAL:
            FATAL_ERROR("Immortal interned string died");
            break;
        default:
            FATAL_ERROR("Inconsistent interned string state");
        }
    }
    a->ob_type->tp_free(a);
}

void string_print(StringObject *a, FILE *stream) {
    if (CHECK_TYPE_STRING(a)) {
        fprintf(stream, "<type 'string'> <value '%s'> <addr %p>\n", a->ob_sval, a);
    }
    else {
        a->ob_type->tp_print((Object *)a, stream);
    }
}

int string_compare(StringObject *lhs, StringObject *rhs) {
    return strcmp(lhs->ob_sval, rhs->ob_sval);
}

long string_hash(StringObject *a) {
    ssize_t len;
    unsigned char *p;
    long x;

    if (a->ob_shash != -1) {
        return a->ob_shash;
    }
    len = a->ob_size;
    p = (unsigned char *)a->ob_sval;
    x = *p << 7;
    while (--len >= 0) {
        x = (1000003 * x) ^ *p++;
    }
    x ^= a->ob_size;
    if (x == -1) {
        x = -2;
    }
    a->ob_shash = x;
    return x;
}

Object *string_to_string(StringObject *a) {
    if (CHECK_TYPE_STRING(a)) {
        INC_REFCNT(a);
        return (Object *)a;
    }
    else {
        return a->ob_type->tp_to_string((Object *)a);
    }
}

ssize_t string_length(StringObject *a) {
    return CHECK_TYPE_STRING(a) ? a->ob_size : a->ob_type->tp_as_sequence->sq_length((Object *)a);
}

Object *string_concat(StringObject *lhs, StringObject *rhs) {
    if (CHECK_TYPE_STRING(lhs) && CHECK_TYPE_STRING(rhs)) {
        StringObject *a;
        ssize_t size = lhs->ob_size + rhs->ob_size;

        a = (StringObject *)gc_malloc(sizeof(StringObject) + size);
        INIT_OBJECT_VAR(a, &type_stringobject, size);
        a->ob_shash = -1;
        a->ob_sstate = SSTATE_NOT_INTERNED;

        memcpy(a->ob_sval, lhs->ob_sval, (size_t)lhs->ob_size);
        memcpy(a->ob_sval + lhs->ob_size, rhs->ob_sval, (size_t)rhs->ob_size);
        a->ob_sval[size] = 0;
        return (Object *)a;
    }
    else {
        return NULL;
    }
}

Object *string_getitem(StringObject *a, ssize_t index) {
    if (CHECK_TYPE_STRING(a)) {
        if (index < 0) {
            index += a->ob_size;
        }
        if (index >= 0 && index < a->ob_size) {
            return string_fromcstring_withsize(&a->ob_sval[index], 1);
        }
        else {
            RUNTIME_EXCEPTION("index out of range");
            return (Object *)none;
        }
    }
    else {
        return (Object *)a->ob_type->tp_as_sequence->sq_getitem((Object *)a, index);
    }
}

Object *string_slice(StringObject *a, ssize_t start, ssize_t end) {
    return NULL;
}

int string_contains(StringObject *a, StringObject *sub) {
    if (CHECK_TYPE_STRING(a) && CHECK_TYPE_STRING(sub)) {
        return strstr(a->ob_sval, sub->ob_sval) != NULL;
    }
    else {
        return 0;
    }
}
