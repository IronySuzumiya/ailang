#pragma once
#ifndef STRING_OBJECT_H
#define STRING_OBJECT_H

#include "../aiconfig.h"

typedef struct _stringobject {
    AiVarObject_HEAD
    long ob_shash;
    int ob_sstate;
    char ob_sval[1];
}
AiStringObject;

enum stringobject_sstate {
    SSTATE_NOT_INTERNED = 0,
    SSTATE_INTERNED_MORTAL,
    SSTATE_INTERNED_IMMORTAL
};

#define CHECK_TYPE_STRING(a) CHECK_FAST_SUBCLASS(a, SUBCLASS_STRING)
#define CHECK_EXACT_TYPE_STRING(a) CHECK_TYPE(a, &type_stringobject)

#define CHECK_STRING_INTERNED(s) (((AiStringObject *)s)->ob_sstate)

#define STRING_AS_CSTRING(s) (((AiStringObject *)s)->ob_sval)

#define STRING_LEN OB_SIZE

#define NULL_STRING (string_from_cstring(""))

#define STRING_EQUAL(lhs, rhs)                                      \
    CHECK_TYPE_STRING(lhs) && CHECK_TYPE_STRING(rhs)                \
        && !strcmp(STRING_AS_CSTRING(lhs), STRING_AS_CSTRING(rhs))

#define STRING_OBJECT_SIZE (offsetof(AiStringObject, ob_sval) + 1)

AiAPI_DATA(AiTypeObject) type_basestringobject;
AiAPI_DATA(AiTypeObject) type_stringobject;
AiAPI_DATA(AiStringObject *) nullstring;
AiAPI_FUNC(AiObject *) string_from_cstring(char *sval);
AiAPI_FUNC(AiObject *) string_from_cstring_with_size(char *sval, ssize_t size);
AiAPI_FUNC(void) string_intern(AiStringObject **a);
AiAPI_FUNC(void) string_intern_immortal(AiStringObject **a);
AiAPI_FUNC(AiObject *) string_join(AiStringObject *internal, AiObject *iter);
AiAPI_FUNC(long) string_hash(AiStringObject *a);
AiAPI_FUNC(AiObject *) string_concat(AiStringObject *lhs, AiStringObject *rhs);
AiAPI_FUNC(AiObject *) string_getitem(AiStringObject *a, ssize_t index);
AiAPI_FUNC(AiObject *) string_slice(AiStringObject *a, ssize_t start, ssize_t end);
AiAPI_FUNC(int) string_contains(AiStringObject *a, AiStringObject *sub);
AiAPI_FUNC(void) string_resize(AiStringObject **a, ssize_t newsize);

#endif
