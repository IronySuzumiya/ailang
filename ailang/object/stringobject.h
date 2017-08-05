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
#define CHECK_EXACT_TYPE_STRING(a) CHECK_TYPE(a, &AiType_String)

#define CHECK_STRING_INTERNED(s) (((AiStringObject *)s)->ob_sstate)

#define STRING_AS_CSTRING(s) (((AiStringObject *)s)->ob_sval)

#define STRING_LEN OB_SIZE

#define NULL_STRING (AiString_From_String(""))

#define STRING_EQUAL(lhs, rhs)                                      \
    CHECK_TYPE_STRING(lhs) && CHECK_TYPE_STRING(rhs)                \
        && !strcmp(STRING_AS_CSTRING(lhs), STRING_AS_CSTRING(rhs))

#define STRING_OBJECT_SIZE (offsetof(AiStringObject, ob_sval) + 1)

AiAPI_DATA(AiTypeObject) AiType_BaseString;
AiAPI_DATA(AiTypeObject) AiType_String;
AiAPI_DATA(AiStringObject *) nullstring;
AiAPI_FUNC(AiObject *) AiString_From_String(char *sval);
AiAPI_FUNC(AiObject *) AiString_From_StringAndSize(char *sval, ssize_t size);
AiAPI_FUNC(void) AiString_Intern(AiStringObject **a);
AiAPI_FUNC(void) AiString_Intern_Immortal(AiStringObject **a);
AiAPI_FUNC(AiObject *) string_join(AiStringObject *internal, AiObject *iter);
AiAPI_FUNC(void) string_resize(AiStringObject **a, ssize_t newsize);
AiAPI_FUNC(long) string_hash(AiStringObject *a);

#endif
