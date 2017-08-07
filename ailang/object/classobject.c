#include "../ailang.h"

AiTypeObject AiType_Class = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "class",                            /* tp_name */
    sizeof(AiClassObject),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    0,//(destructor)class_dealloc,          /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_hash */
    AiInstance_New,                     /* tp_call */
    0,//class_str,                          /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//(getattrofunc)class_getattr,        /* tp_getattro */
    0,//(setattrofunc)class_setattr,        /* tp_setattro */

    0,                                  /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,//class_new,                          /* tp_new */
};

AiTypeObject AiType_Instance = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "instance",                         /* tp_name */
    sizeof(AiInstanceObject),           /* tp_basicsize */
    0,                                  /* tp_itemsize */
    0,//(destructor)instance_dealloc,       /* tp_dealloc */
    0,                                  /* tp_print */
    0,//instance_compare,                   /* tp_compare */
    
    0,//&instance_as_number,                /* tp_as_number */
    0,//&instance_as_sequence,              /* tp_as_sequence */
    0,//&instance_as_mapping,               /* tp_as_mapping */

    0,//(hashfunc)instance_hash,            /* tp_hash */
    0,//instance_call,                      /* tp_call */
    0,//(unaryfunc)instance_str,            /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//(getattrofunc)instance_getattr,     /* tp_getattro */
    0,//(setattrofunc)instance_setattr,     /* tp_setattro */

    0,                                  /* tp_flags */

    0,//(unaryfunc)instance_getiter,        /* tp_iter */
    0,//(unaryfunc)instance_iternext,       /* tp_iternext */

    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,//instance_new,                       /* tp_new */
};

AiTypeObject AiType_Method = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "instancemethod",                   /* tp_name */
    sizeof(AiMethodObject),             /* tp_basicsize */
    0,                                  /* tp_itemsize */
    0,//(destructor)instancemethod_dealloc, /* tp_dealloc */
    0,                                  /* tp_print */
    0,//(cmpfunc)instancemethod_compare,    /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,//(hashfunc)instancemethod_hash,      /* tp_hash */
    0,//instancemethod_call,                /* tp_call */
    0,                                  /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//instancemethod_getattr,             /* tp_getattro */
    0,//AiObject_Generic_Setattr,           /* tp_setattro */

    0,                                  /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_methods */
    0,//instancemethod_memberlist,          /* tp_members */
    0,//instancemethod_getset,              /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//instancemethod_descr_get,           /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,//instancemethod_new,                 /* tp_new */
};
