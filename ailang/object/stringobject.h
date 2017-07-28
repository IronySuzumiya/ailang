#pragma once
#ifndef STRING_OBJECT_H
#define STRING_OBJECT_H

#include "../system/utils.h"

typedef struct _stringobject {
    OBJECT_VAR_HEAD
    long ob_shash;
    int ob_sstate;
    char ob_sval[1];
}
StringObject;

enum stringobject_sstate {
    SSTATE_NOT_INTERNED = 0,
    SSTATE_INTERNED_MORTAL,
    SSTATE_INTERNED_IMMORTAL
};

#define CHECK_TYPE_STRING(a) CHECK_TYPE(a, &type_stringobject)

#define CHECK_STRING_INTERNED(s) (((StringObject *)s)->ob_sstate)

API_DATA(TypeObject) type_stringobject;
API_DATA(StringObject *) nullstring;
API_FUNC(Object *) string_fromcstring(const char *sval);
API_FUNC(void) string_intern(StringObject **a);
API_FUNC(void) string_intern_immortal(StringObject **a);

#endif
