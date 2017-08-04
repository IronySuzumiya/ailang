#include "../ailang.h"

static void object_dealloc(AiObject *ob);

AiTypeObject type_noneobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "nonetype",                     /* tp_name */
};

AiTypeObject type_notimplementedobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "NotImplemented",
};

AiTypeObject type_baseobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "object",                           /* tp_name */
    sizeof(AiObject),                   /* tp_basicsize */
    0,                                  /* tp_itemsize */
    object_dealloc,                     /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    (hashfunc)pointer_hash,             /* tp_hash */
    0,                                  /* tp_call */
    0,//object_str,                         /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//object_generic_getattr,             /* tp_getattro */
    0,//object_generic_setattr,             /* tp_setattro */

    BASE_TYPE,                          /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,//object_methods,                     /* tp_methods */
    0,                                  /* tp_members */
    0,//object_getsetlist,                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,//object_init,                        /* tp_init */
    0,//type_generic_alloc,                 /* tp_alloc */
    0,//object_new,                         /* tp_new */
    AiObject_GC_DEL,                    /* tp_free */
};

AiObject none = {
    INIT_AiObject_HEAD(&type_noneobject)
};

AiObject notimplemented = {
    INIT_AiObject_HEAD(&type_notimplementedobject)
};

void object_dealloc(AiObject *ob) {
    OB_FREE(ob);
}
