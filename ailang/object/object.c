#include "../ailang.h"

static void none_print(AiNoneObject *ob, FILE *stream);

AiTypeObject type_noneobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "none",                         /* tp_name */
    0,                              /* tp_dealloc */
    (printfunc)none_print,          /* tp_print */
    0,                              /* tp_compare */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    (hashfunc)pointer_hash,         /* tp_hash */
    0,                              /* tp_to_string */
    0,                              /* tp_free */
};

AiNoneObject _none = {
    INIT_OBJECT_HEAD(&type_noneobject)
};

AiNoneObject *none = &_none;

long pointer_hash(void *p) {
    long x;
    size_t y = (size_t)p;
    y = (y >> 4) | (y << (8 * sizeof(void *) - 4));
    x = (long)y;
    return x == -1 ? -2 : x;
}

void none_print(AiNoneObject *ob, FILE *stream) {
    if (CHECK_TYPE_NONE(ob)) {
        fprintf(stream, "<type 'none'> <addr %p>\n", ob);
    }
    else {
        ob->ob_type->tp_print((AiObject *)ob, stream);
    }
}
