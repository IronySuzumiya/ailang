#include "../ailang.h"

AiTypeObject type_methoddescrobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "method_descriptor",                    /* tp_name */
    sizeof(AiMethodDescrObject),            /* tp_basicsize */
    0,                                      /* tp_itemsize */
    0,//(destructor)descr_dealloc,              /* tp_dealloc */
    0,                                      /* tp_print */
    0,                                      /* tp_compare*/

    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */

    0,                                      /* tp_hash */
    0,//(ternaryfunc)methoddescr_call,          /* tp_call */
    0,                                      /* tp_str */

    0,                                      /* tp_getattr */
    0,                                      /* tp_setattr */
    0,//object_generic_getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */
    0,//method_getset,                          /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)method_get,               /* tp_descr_get */
    0,                                      /* tp_descr_set */
};

AiTypeObject type_classmethoddescrobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "classmethod_descriptor",               /* tp_name */
    sizeof(AiMethodDescrObject),            /* tp_basicsize */
    0,                                      /* tp_itemsize */
    0,//(destructor)descr_dealloc,              /* tp_dealloc */
    0,                                      /* tp_print */
    0,                                      /* tp_compare */
    
    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */

    0,                                      /* tp_hash */
    0,//(ternaryfunc)classmethoddescr_call,     /* tp_call */
    0,                                      /* tp_str */

    0,                                      /* tp_getattr */
    0,                                      /* tp_setattr */
    0,//object_generic_getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */
    0,//method_getset,                          /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)classmethod_get,          /* tp_descr_get */
    0,                                      /* tp_descr_set */
};

AiTypeObject type_memberdescrobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "member_descriptor",                    /* tp_name */
    sizeof(AiMemberDescrObject),            /* tp_basicsize */
    0,                                      /* tp_itemsize */
    0,//(destructor)descr_dealloc,              /* tp_dealloc */
    0,                                      /* tp_print */
    0,                                      /* tp_compare */

    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */

    0,                                      /* tp_hash */
    0,                                      /* tp_call */
    0,                                      /* tp_str */

    0,                                      /* tp_getattr */
    0,                                      /* tp_setattr */
    0,//object_generic_getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */
    0,//member_getset,                          /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)member_get,               /* tp_descr_get */
    0,//(descrsetfunc)member_set,               /* tp_descr_set */
};

AiTypeObject type_getsetdescr = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "getset_descriptor",                    /* tp_name */
    sizeof(AiGetSetDescrObject),            /* tp_basicsize */
    0,                                      /* tp_itemsize */
    0,//(destructor)descr_dealloc,              /* tp_dealloc */
    0,                                      /* tp_print */
    0,                                      /* tp_compare */

    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */

    0,                                      /* tp_hash */
    0,                                      /* tp_call */
    0,                                      /* tp_str */

    0,                                      /* tp_getattr */
    0,                                      /* tp_setattr */
    0,//object_generic_getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */
    0,//getset_getset,                          /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)getset_get,               /* tp_descr_get */
    0,//(descrsetfunc)getset_set,               /* tp_descr_set */
};

AiTypeObject type_wrapperdescrobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "wrapper_descriptor",                   /* tp_name */
    sizeof(AiWrapperDescrObject),           /* tp_basicsize*/
    0,                                      /* tp_itemsize */
    0,//(destructor)descr_dealloc,              /* tp_dealloc */
    0,                                      /* tp_print */
    0,                                      /* tp_compare */
    
    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */

    0,                                      /* tp_hash */
    0,//(ternaryfunc)wrapperdescr_call,         /* tp_call */
    0,                                      /* tp_str */

    0,                                      /* tp_getattr */
    0,                                      /* tp_setattr */
    0,//object_generic_getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */
    0,//wrapperdescr_getset,                    /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)wrapperdescr_get,         /* tp_descr_get */
    0,                                      /* tp_descr_set */
};

AiObject *descr_newwrapper(AiTypeObject *type, struct wrapperbase *base, void *wrapped) {
    AiWrapperDescrObject *descr;
    descr = (AiWrapperDescrObject *)descr_new(&type_wrapperdescrobject, type, base->name);
    if (descr) {
        descr->d_base = base;
        descr->d_wrapped = wrapped;
    }
    return (AiObject *)descr;
}
