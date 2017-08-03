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
    0,                              /* tp_tostring */
    0,                              /* tp_free */
};

AiNoneObject _none = {
    INIT_OBJECT_HEAD(&type_noneobject)
};

AiNoneObject *none = &_none;

void none_print(AiNoneObject *ob, FILE *stream) {

}
