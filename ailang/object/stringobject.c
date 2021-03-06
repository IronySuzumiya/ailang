#include "../ailang.h"

static AiStringObject *characters[UCHAR_MAX + 1];
static AiDictObject *interned;

static AiObject *_AiString_From_StringAndSize(char *sval, ssize_t size);
static AiObject *string_concat(AiStringObject *lhs, AiStringObject *rhs);
static AiObject *string_getitem(AiStringObject *a, ssize_t index);
static AiObject *string_slice(AiStringObject *a, ssize_t start, ssize_t end);
static int string_contains(AiStringObject *a, AiStringObject *sub);
static void string_dealloc(AiStringObject *a);
static int string_compare(AiStringObject *lhs, AiStringObject *rhs);
static AiObject *string_str(AiStringObject *a);
static ssize_t string_length(AiStringObject *a);

AiTypeObject AiType_BaseString = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "basestring",                   /* tp_name */
    0,                              /* tp_basesize */
    0,                              /* tp_itemsize */

    0,                              /* tp_dealloc */
    0,                              /* tp_compare */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */

    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */

    //&AiBaseObject,                  /* tp_base */
    0,                              /* tp_dict */
    //basestring_new,                 /* tp_new */
};

// string object is immutable
static AiSequenceMethods string_as_sequence = {
    (lengthfunc)string_length,
    (binaryfunc)string_concat,
    (ssizeargfunc)string_getitem,
    0,
    (ssize2argfunc)string_slice,
    (enquiry2)string_contains,
};

AiTypeObject AiType_String = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "string",                                   /* tp_name */
    STRING_OBJECT_SIZE,                         /* tp_basicsize */
    sizeof(char),                               /* tp_itemsize */

    (destructor)string_dealloc,                 /* tp_dealloc */
    (cmpfunc)string_compare,                    /* tp_compare */
    (hashfunc)string_hash,                      /* tp_hash */
    0,                                          /* tp_call */
    (unaryfunc)string_str,                      /* tp_str */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */

    0,                                          /* tp_as_number */
    &string_as_sequence,                        /* tp_as_sequence */
    0,                                          /* tp_as_mapping */

    &AiType_BaseString,                         /* tp_base */
    0,                                          /* tp_dict */
    0,//string_new,                                 /* tp_new */
    0,                                          /* tp_init */
    AiObject_Del,                               /* tp_free */
};

AiStringObject *nullstring;

AiObject *AiString_From_String(char *sval) {
    return _AiString_From_StringAndSize(sval, strlen(sval));
}

AiObject *AiString_From_StringAndSize(char *sval, ssize_t size) {
    if (size < 0)
        size = 0;
    return _AiString_From_StringAndSize(sval, size);
}

void AiString_Intern(AiStringObject **a) {
    AiObject *s = (AiObject *)(*a);
    AiObject *t;

    if (!CHECK_EXACT_TYPE_STRING(s) || CHECK_STRING_INTERNED(s)) {
        return;
    }

    if (!interned) {
        interned = (AiDictObject *)AiDict_New();
    }

    if (t = AiDict_GetItem(interned, s)) {
        INC_REFCNT(t);
        DEC_REFCNT(*a);
        *a = (AiStringObject *)t;
    }
    else {
        AiDict_SetItem(interned, s, s);
        s->ob_refcnt -= 2;
        CHECK_STRING_INTERNED(s) = SSTATE_INTERNED_MORTAL;
    }
}

void AiString_Intern_Immortal(AiStringObject **a) {
    AiString_Intern(a);
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

    if (CHECK_EXACT_TYPE_STRING(split)) {
        intersize = STRING_LEN(split);
    }
    else {
        RUNTIME_EXCEPTION("only string expected");
        return NULL;
    }

    if (CHECK_EXACT_TYPE_LIST(list)) {
        if (list->ob_item && LIST_SIZE(list) > 0) {
            if (!CHECK_EXACT_TYPE_STRING(list->ob_item[0])) {
                RUNTIME_EXCEPTION("sequence item 0: only string expected");
                return NULL;
            }
            size = STRING_LEN(list->ob_item[0]);
            for (ssize_t i = 1; i < LIST_SIZE(list); ++i) {
                if (!CHECK_EXACT_TYPE_STRING(list->ob_item[i])) {
                    RUNTIME_EXCEPTION("sequence item %d: only string expected", i);
                    return NULL;
                }
                size += intersize + STRING_LEN(list->ob_item[i]);
            }

            str = AiString_From_StringAndSize(NULL, size);
            p = STRING_AS_CSTRING(str);
            AiMem_Copy(p, STRING_AS_CSTRING(list->ob_item[0]), STRING_LEN(list->ob_item[0]));
            p += STRING_LEN(list->ob_item[0]);
            for (ssize_t i = 1; i < LIST_SIZE(list); ++i) {
                AiMem_Copy(p, STRING_AS_CSTRING(split), intersize);
                p += intersize;
                AiMem_Copy(p, STRING_AS_CSTRING(list->ob_item[i]), STRING_LEN(list->ob_item[i]));
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
        return NULL;
    }
}

AiObject *_AiString_From_StringAndSize(char *sval, ssize_t size) {
    AiStringObject *a;

    if (size == 0 && (a = nullstring)) {
        return (AiObject *)nullstring;
    }
    else if (size == 1 && (a = characters[*sval & UCHAR_MAX])) {
        return (AiObject *)a;
    }
    else {
        a = AiVarObject_NEW(AiStringObject, &AiType_String, size);
        a->ob_shash = -1;
        a->ob_sstate = SSTATE_NOT_INTERNED;
        if (sval)
            AiMem_Copy(STRING_AS_CSTRING(a), sval, size + 1);
        else
            AiMem_Set(STRING_AS_CSTRING(a), 0, size + 1);
        if (size == 0) {
            AiString_Intern(&a);
            nullstring = a;
            INC_REFCNT(a);
        }
        else if (size == 1) {
            AiString_Intern(&a);
            characters[*sval & UCHAR_MAX] = a;
            INC_REFCNT(a);
        }
        return (AiObject *)a;
    }
}

void string_dealloc(AiStringObject *a) {
    if (CHECK_EXACT_TYPE_STRING(a)) {
        switch (CHECK_STRING_INTERNED(a))
        {
        case SSTATE_NOT_INTERNED:
            break;
        case SSTATE_INTERNED_MORTAL:
            a->ob_refcnt = 3;
            if (AiDict_DelItem(interned, (AiObject *)a)) {
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

AiObject *string_str(AiStringObject *a) {
    if (CHECK_EXACT_TYPE_STRING(a)) {
        INC_REFCNT(a);
        return (AiObject *)a;
    }
    else {
        return OB_TO_STRING(a);
    }
}

ssize_t string_length(AiStringObject *a) {
    return CHECK_EXACT_TYPE_STRING(a) ? STRING_LEN(a) : a->ob_type->tp_as_sequence->sq_length((AiObject *)a);
}

AiObject *string_concat(AiStringObject *lhs, AiStringObject *rhs) {
    if (CHECK_EXACT_TYPE_STRING(lhs) && CHECK_EXACT_TYPE_STRING(rhs)) {
        AiStringObject *a;
        ssize_t size = STRING_LEN(lhs) + STRING_LEN(rhs);

        a = AiVarObject_NEW(AiStringObject, &AiType_String, size);
        a->ob_shash = -1;
        a->ob_sstate = SSTATE_NOT_INTERNED;

        AiMem_Copy(STRING_AS_CSTRING(a), STRING_AS_CSTRING(lhs), STRING_LEN(lhs));
        AiMem_Copy(STRING_AS_CSTRING(a) + STRING_LEN(lhs), STRING_AS_CSTRING(rhs), STRING_LEN(rhs));
        STRING_AS_CSTRING(a)[size] = 0;
        return (AiObject *)a;
    }
    else {
        UNSUPPORTED_CONCAT(OB_TYPENAME(lhs), OB_TYPENAME(rhs));
        return NULL;
    }
}

AiObject *string_getitem(AiStringObject *a, ssize_t index) {
    MAKE_INDEX_IN_RANGE(index, STRING_LEN(a));
    if (index < STRING_LEN(a)) {
        return AiString_From_StringAndSize(&STRING_AS_CSTRING(a)[index], 1);
    }
    else {
        RUNTIME_EXCEPTION("index out of range");
        return NULL;
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
            return AiString_From_StringAndSize(&STRING_AS_CSTRING(a)[start], end - start);
        }
    }
    else {
        RUNTIME_EXCEPTION("invalid range sliced");
        return NULL;
    }
}

int string_contains(AiStringObject *a, AiStringObject *sub) {
    if (CHECK_EXACT_TYPE_STRING(a) && CHECK_EXACT_TYPE_STRING(sub)) {
        return strstr(STRING_AS_CSTRING(a), STRING_AS_CSTRING(sub)) != NULL;
    }
    else {
        UNSUPPORTED_CONTAINS(OB_TYPENAME(a), OB_TYPENAME(sub));
        return 0;
    }
}

void string_resize(AiStringObject **a, ssize_t newsize) {
    if (CHECK_EXACT_TYPE_STRING(*a)) {
        switch (CHECK_STRING_INTERNED(*a))
        {
        case SSTATE_NOT_INTERNED:
            break;
        case SSTATE_INTERNED_MORTAL:
            (*a)->ob_refcnt = 3;
            if (AiDict_DelItem(interned, (AiObject *)(*a))) {
                FATAL_ERROR("deletion of interned string failed");
            }
            break;
        case SSTATE_INTERNED_IMMORTAL:
            break;
        default:
            FATAL_ERROR("Inconsistent interned string state");
        }
    }
    *a = AiMem_Realloc(*a, sizeof(AiStringObject) + newsize);
    INIT_REFCNT(*a);
    STRING_LEN(*a) = newsize;
    STRING_AS_CSTRING(*a)[newsize] = 0;
    (*a)->ob_shash = -1;
}
