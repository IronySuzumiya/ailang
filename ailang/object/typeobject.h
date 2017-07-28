#pragma once
#ifndef TYPE_OBJECT_H
#define TYPE_OBJECT_H

#include "../system/utils.h"

typedef struct _typeobject {
    OBJECT_VAR_HEAD
    char *tp_name;

    destructor tp_dealloc;
    printfunc tp_print;
    cmpfunc tp_compare;

    numbermethods *tp_as_number;
    sequencemethods *tp_as_sequence;
    mappingmethods *tp_as_mapping;

    hashfunc tp_hash;
    unaryfunc tp_to_string;
    freefunc tp_free;
}
TypeObject;

#define CHECK_TYPE_TYPE(a) CHECK_TYPE(a, &type_typeobject)

API_DATA(TypeObject) type_typeobject;

#endif
