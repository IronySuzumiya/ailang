#include "../ailang.h"

static void type_print(AiTypeObject *ob, FILE *stream);
static AiObject *type_tostring(AiTypeObject *ob);

AiTypeObject type_typeobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "type",                         /* tp_name */
    0,                              /* tp_dealloc */
    (printfunc)type_print,          /* tp_print */
    0,                              /* tp_compare */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    (hashfunc)pointer_hash,         /* tp_hash */
    (unaryfunc)type_tostring,      /* tp_tostring */
    0,                              /* tp_free */
};

void type_print(AiTypeObject *ob, FILE *stream) {
    fputs("<type 'type'>", stream);
}

AiObject *type_tostring(AiTypeObject *ob) {
    return CHECK_TYPE(ob, &type_typeobject) ? string_from_cstring("<type 'type'>") : OB_TO_STRING(ob);
}
