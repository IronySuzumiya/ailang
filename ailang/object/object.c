#include "../ailang.h"

static void object_dealloc(AiObject *ob);

AiTypeObject AiType_None = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "nonetype",                         /* tp_name */
};

AiTypeObject AiType_NotImplemented = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "NotImplemented",                   /* tp_name */
};

AiTypeObject AiType_BaseObject = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "object",                           /* tp_name */
    sizeof(AiObject),                   /* tp_basicsize */
    0,                                  /* tp_itemsize */

    object_dealloc,                     /* tp_dealloc */
    0,                                  /* tp_compare */
    (hashfunc)Pointer_Hash,             /* tp_hash */
    0,                                  /* tp_call */
    0,//object_str,                         /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//object_new,                         /* tp_new */
    0,//object_init,                        /* tp_init */
    AiObject_Del,                    /* tp_free */
};

AiObject none = {
    AiObject_HEAD_INIT(&AiType_None)
};

AiObject notimplemented = {
    AiObject_HEAD_INIT(&AiType_NotImplemented)
};

void object_dealloc(AiObject *ob) {
    OB_FREE(ob);
}
