#include "../ailang.h"

static AiStringObject *characters[UCHAR_MAX + 1];
static AiDictObject *interned;

static AiObject *_string_from_cstring_with_size(char *sval, ssize_t size);
static void string_dealloc(AiStringObject *a);
static void string_print(AiStringObject *a, FILE *stream);
static int string_compare(AiStringObject *lhs, AiStringObject *rhs);
static AiObject *string_to_string(AiStringObject *a);
static void string_free(void *p);
static ssize_t string_length(AiStringObject *a);

static numbermethods string_as_number = {
    (binaryfunc)string_concat,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,

    0,

    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
};

// string object is immutable
static sequencemethods string_as_sequence = {
    (lengthfunc)string_length,
    (binaryfunc)string_concat,
    (ssizeargfunc)string_getitem,
    0,
    (ssize2argfunc)string_slice,
    (enquiry2)string_contains,
};

AiTypeObject type_stringobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "string",                                   /* tp_name */
    (destructor)string_dealloc,                 /* tp_dealloc */
    (printfunc)string_print,                    /* tp_print */
    (cmpfunc)string_compare,                    /* tp_compare */

    &string_as_number,                          /* tp_as_number */
    &string_as_sequence,                        /* tp_as_sequence */
    0,                                          /* tp_as_mapping */

    (hashfunc)string_hash,                      /* tp_hash */
    (unaryfunc)string_to_string,                /* tp_to_string */
    (freefunc)string_free,                      /* tp_free */
};

AiStringObject *nullstring;

AiObject *string_from_cstring(char *sval) {
    return _string_from_cstring_with_size(sval, strlen(sval));
}

AiObject *string_from_cstring_with_size(char *sval, ssize_t size) {
    if (size < 0)
        size = 0;
    return _string_from_cstring_with_size(sval, size);
}

void string_intern(AiStringObject **a) {
    AiObject *s = (AiObject *)(*a);
    AiObject *t;

    if (!CHECK_TYPE_STRING(s) || CHECK_STRING_INTERNED(s)) {
        return;
    }

    if (!interned) {
        interned = (AiDictObject *)dict_new();
    }

    if (t = dict_getitem(interned, s)) {
        INC_REFCNT(t);
        DEC_REFCNT(*a);
        *a = (AiStringObject *)t;
    }
    else {
        dict_setitem(interned, s, s);
        s->ob_refcnt -= 2;
        CHECK_STRING_INTERNED(s) = SSTATE_INTERNED_MORTAL;
    }
}

void string_intern_immortal(AiStringObject **a) {
    string_intern(a);
    if (CHECK_STRING_INTERNED(*a) != SSTATE_INTERNED_IMMORTAL) {
        CHECK_STRING_INTERNED(*a) = SSTATE_INTERNED_IMMORTAL;
        INC_REFCNT(*a);
    }
}

AiObject *string_join(AiStringObject *split, AiObject *iter) {
    AiListObject *list = (AiListObject *)iter;
    AiObject *str;
    ssize_t intersize;
    ssize_t size;
    char *p;

    if (CHECK_TYPE_STRING(split)) {
        intersize = STRING_LEN(split);
    }
    else {
        RUNTIME_EXCEPTION("only string expected");
        return NONE;
    }

    if (CHECK_TYPE_LIST(list)) {
        if (list->ob_item && LIST_SIZE(list) > 0) {
            if (!CHECK_TYPE_STRING(list->ob_item[0])) {
                RUNTIME_EXCEPTION("sequence item 0: only string expected");
                return NONE;
            }
            size = STRING_LEN(list->ob_item[0]);
            for (ssize_t i = 1; i < LIST_SIZE(list); ++i) {
                if (!CHECK_TYPE_STRING(list->ob_item[i])) {
                    RUNTIME_EXCEPTION("sequence item %d: only string expected", i);
                    return NONE;
                }
                size += intersize + STRING_LEN(list->ob_item[i]);
            }

            str = string_from_cstring_with_size(NULL, size);
            p = STRING_AS_CSTRING(str);
            AiMEM_COPY(p, STRING_AS_CSTRING(list->ob_item[0]), STRING_LEN(list->ob_item[0]));
            p += STRING_LEN(list->ob_item[0]);
            for (ssize_t i = 1; i < LIST_SIZE(list); ++i) {
                AiMEM_COPY(p, STRING_AS_CSTRING(split), intersize);
                p += intersize;
                AiMEM_COPY(p, STRING_AS_CSTRING(list->ob_item[i]), STRING_LEN(list->ob_item[i]));
                p += STRING_LEN(list->ob_item[i]);
            }
            *p = 0;

            return str;
        }
        else {
            return NULL_STRING;
        }
    }
    else {
        RUNTIME_EXCEPTION("only list now, iterator would be supported in the future");
        return NONE;
    }
}

AiObject *_string_from_cstring_with_size(char *sval, ssize_t size) {
    AiStringObject *a;

    if (size == 0 && (a = nullstring)) {
        return (AiObject *)nullstring;
    }
    else if (size == 1 && (a = characters[*sval & UCHAR_MAX])) {
        return (AiObject *)a;
    }
    else {
        a = AiMEM_ALLOC(sizeof(AiStringObject) + size);
        INIT_OBJECT_VAR(a, &type_stringobject, size);
        a->ob_shash = -1;
        a->ob_sstate = SSTATE_NOT_INTERNED;
        if (sval)
            AiMEM_COPY(STRING_AS_CSTRING(a), sval, size + 1);
        else
            AiMEM_SET(STRING_AS_CSTRING(a), 0, size + 1);
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
        return (AiObject *)a;
    }
}

void string_dealloc(AiStringObject *a) {
    if (CHECK_TYPE_STRING(a)) {
        switch (CHECK_STRING_INTERNED(a))
        {
        case SSTATE_NOT_INTERNED:
            break;
        case SSTATE_INTERNED_MORTAL:
            a->ob_refcnt = 3;
            if (dict_delitem(interned, (AiObject *)a)) {
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
    OB_FREE(a);
}

void string_print(AiStringObject *a, FILE *stream) {
    fputs(STRING_AS_CSTRING(a), stream);
}

int string_compare(AiStringObject *lhs, AiStringObject *rhs) {
    return string_hash(lhs) == string_hash(rhs)
        && strcmp(STRING_AS_CSTRING(lhs), STRING_AS_CSTRING(rhs));
}

long string_hash(AiStringObject *a) {
    ssize_t len;
    unsigned char *p;
    long x;

    if (a->ob_shash != -1) {
        return a->ob_shash;
    }
    len = STRING_LEN(a);
    p = (unsigned char *)STRING_AS_CSTRING(a);
    x = *p << 7;
    while (--len >= 0) {
        x = (1000003 * x) ^ *p++;
    }
    x ^= STRING_LEN(a);
    if (x == -1) {
        x = -2;
    }
    a->ob_shash = x;
    return x;
}

AiObject *string_to_string(AiStringObject *a) {
    if (CHECK_TYPE_STRING(a)) {
        AiStringObject *str;
        ssize_t size = STRING_LEN(a) + 2;
        str = (AiStringObject *)AiMEM_ALLOC(sizeof(AiStringObject) + size);
        INIT_OBJECT_VAR(str, &type_stringobject, size);
        str->ob_shash = -1;
        str->ob_sstate = SSTATE_NOT_INTERNED;

        STRING_AS_CSTRING(str)[0] = '\'';
        AiMEM_COPY(&STRING_AS_CSTRING(str)[1], STRING_AS_CSTRING(a), STRING_LEN(a));
        STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = '\'';
        STRING_AS_CSTRING(str)[STRING_LEN(str)] = 0;
        return (AiObject *)str;
    }
    else {
        return OB_TO_STRING(a);
    }
}

void string_free(void *p) {
    AiMEM_FREE(p);
}

ssize_t string_length(AiStringObject *a) {
    return CHECK_TYPE_STRING(a) ? STRING_LEN(a) : a->ob_type->tp_as_sequence->sq_length((AiObject *)a);
}

AiObject *string_concat(AiStringObject *lhs, AiStringObject *rhs) {
    if (CHECK_TYPE_STRING(lhs) && CHECK_TYPE_STRING(rhs)) {
        AiStringObject *a;
        ssize_t size = STRING_LEN(lhs) + STRING_LEN(rhs);

        a = (AiStringObject *)AiMEM_ALLOC(sizeof(AiStringObject) + size);
        INIT_OBJECT_VAR(a, &type_stringobject, size);
        a->ob_shash = -1;
        a->ob_sstate = SSTATE_NOT_INTERNED;

        AiMEM_COPY(STRING_AS_CSTRING(a), STRING_AS_CSTRING(lhs), STRING_LEN(lhs));
        AiMEM_COPY(STRING_AS_CSTRING(a) + STRING_LEN(lhs), STRING_AS_CSTRING(rhs), STRING_LEN(rhs));
        STRING_AS_CSTRING(a)[size] = 0;
        return (AiObject *)a;
    }
    else {
        UNSUPPORTED_CONCAT(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NONE;
    }
}

AiObject *string_getitem(AiStringObject *a, ssize_t index) {
    MAKE_INDEX_IN_RANGE(index, STRING_LEN(a));
    if (index < STRING_LEN(a)) {
        return string_from_cstring_with_size(&STRING_AS_CSTRING(a)[index], 1);
    }
    else {
        RUNTIME_EXCEPTION("index out of range");
        return NONE;
    }
}

AiObject *string_slice(AiStringObject *a, ssize_t start, ssize_t end) {
    MAKE_INDEX_IN_RANGE(start, STRING_LEN(a));
    MAKE_INDEX_IN_RANGE(end, STRING_LEN(a));
    if (start < STRING_LEN(a) && start < end) {
        if (end > STRING_LEN(a)) {
            end = STRING_LEN(a);
        }
        if (start == 0 && end == STRING_LEN(a)) {
            INC_REFCNT(a);
            return (AiObject *)a;
        }
        else {
            return string_from_cstring_with_size(&STRING_AS_CSTRING(a)[start], end - start);
        }
    }
    else {
        RUNTIME_EXCEPTION("invalid range sliced");
        return NONE;
    }
}

int string_contains(AiStringObject *a, AiStringObject *sub) {
    if (CHECK_TYPE_STRING(a) && CHECK_TYPE_STRING(sub)) {
        return strstr(STRING_AS_CSTRING(a), STRING_AS_CSTRING(sub)) != NULL;
    }
    else {
        UNSUPPORTED_CONTAINS(OB_TYPENAME(a), OB_TYPENAME(sub));
        return 0;
    }
}

void string_resize(AiStringObject **a, ssize_t newsize) {
    if (CHECK_TYPE_STRING(*a)) {
        switch (CHECK_STRING_INTERNED(*a))
        {
        case SSTATE_NOT_INTERNED:
            break;
        case SSTATE_INTERNED_MORTAL:
            (*a)->ob_refcnt = 3;
            if (dict_delitem(interned, (AiObject *)(*a))) {
                FATAL_ERROR("deletion of interned string failed");
            }
            break;
        case SSTATE_INTERNED_IMMORTAL:
            break;
        default:
            FATAL_ERROR("Inconsistent interned string state");
        }
    }
    *a = AiMEM_REALLOC(*a, sizeof(AiStringObject) + newsize);
    INIT_REFCNT(*a);
    STRING_LEN(*a) = newsize;
    STRING_AS_CSTRING(*a)[newsize] = 0;
    (*a)->ob_shash = -1;
}
