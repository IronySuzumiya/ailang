#include "../ailang.h"

static AiDescrObject *descr_new(AiTypeObject *descrtype, AiTypeObject *type, char *name);

AiTypeObject AiType_MethodDescr = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
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

    0,//AiObject_Generic_Getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */

    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)method_get,               /* tp_descr_get */
    0,                                      /* tp_descr_set */
};

AiTypeObject AiType_MemberDescr = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
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

    0,//AiObject_Generic_Getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */

    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)member_get,               /* tp_descr_get */
    0,//(descrsetfunc)member_set,               /* tp_descr_set */
};

AiTypeObject AiType_WrapperDescr = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
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

    0,//AiObject_Generic_Getattr,                 /* tp_getattro */
    0,                                      /* tp_setattro */

    0,                                      /* tp_flags */

    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */

    0,                                      /* tp_methods */
    0,//descr_members,                          /* tp_members */

    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,//(descrgetfunc)wrapperdescr_get,         /* tp_descr_get */
    0,                                      /* tp_descr_set */
};

AiDescrObject *descr_new(AiTypeObject *descrtype, AiTypeObject *type, char *name) {
    AiDescrObject *descr;
    descr = (AiDescrObject *)AiType_Generic_Alloc(descrtype, 0);
    XINC_REFCNT(type);
    descr->d_type = type;
    descr->d_name = AiString_From_String(name);
    AiString_Intern((AiStringObject **)&descr->d_name);
    return descr;
}

AiObject *AiDescr_NewWrapper(AiTypeObject *type, struct wrapperbase *base, void *wrapped) {
    AiWrapperDescrObject *descr;
    descr = (AiWrapperDescrObject *)descr_new(&AiType_WrapperDescr, type, base->name);
    descr->d_base = base;
    descr->d_wrapped = wrapped;
    return (AiObject *)descr;
}

AiObject *AiDescr_NewMember(AiTypeObject *type, AiMemberDef *member) {
    AiMemberDescrObject *descr;
    descr = (AiMemberDescrObject *)descr_new(&AiType_MemberDescr, type, member->name);
    descr->d_member = member;
    return (AiObject *)descr;
}

AiObject *AiDescr_NewMethod(AiTypeObject *type, AiMethodDef *method) {
    AiMethodDescrObject *descr;
    descr = (AiMethodDescrObject *)descr_new(&AiType_MethodDescr, type, method->ml_name);
    descr->d_method = method;
    return (AiObject *)descr;
}
