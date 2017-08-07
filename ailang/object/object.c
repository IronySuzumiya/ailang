#include "../ailang.h"

static void object_dealloc(AiObject *ob);

static AiMethodDef object_methods[] = {
    { NULL }
};

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
    0,                                  /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    (hashfunc)Pointer_Hash,             /* tp_hash */
    0,                                  /* tp_call */
    0,//object_str,                         /* tp_str */

    0,//AiObject_Generic_Getattr,             /* tp_getattro */
    0,//AiObject_Generic_Setattr,             /* tp_setattro */

    BASE_TYPE,                          /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    object_methods,                     /* tp_methods */
    0,                                  /* tp_members */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,//object_init,                        /* tp_init */
    0,//AiType_Generic_Alloc,                 /* tp_alloc */
    0,//object_new,                         /* tp_new */
    AiObject_GC_Del,                    /* tp_free */
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
