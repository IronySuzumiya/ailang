#include "../ailang.h"

static void type_dealloc(AiTypeObject *type);
static void type_print(AiTypeObject *ob, FILE *stream);
static AiObject *type_str(AiTypeObject *ob);

AiTypeObject type_typeobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "type",                             /* tp_name */
    sizeof(AiHeapTypeObject),           /* tp_basicsize */
    0,//sizeof(AiMemberDef),            /* tp_itemsize */
    (destructor)type_dealloc,           /* tp_dealloc */
    (printfunc)type_print,              /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    (hashfunc)pointer_hash,             /* tp_hash */
    0,//(ternaryfunc)type_call,             /* tp_call */
    (unaryfunc)type_str,                /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//(getattrofunc)type_getattro,        /* tp_getattro */
    0,//(setattrofunc)type_setattro,        /* tp_setattro */

    SUBCLASS_TYPE | BASE_TYPE,          /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,//type_methods,                       /* tp_methods */
    0,//type_members,                       /* tp_members */
    0,//type_getset,                        /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    offsetof(AiTypeObject, tp_dict),    /* tp_dictoffset */
    0,//type_init,                          /* tp_init */
    0,                                  /* tp_alloc */
    0,//type_new,                           /* tp_new */
    AiObject_GC_DEL,                    /* tp_free */
    0,//tp_is_gc,                           /* tp_is_gc */
};

void type_dealloc(AiTypeObject *type) {
    AiHeapTypeObject *et;

    assert(type->tp_flags & HEAP_TYPE);
    et = (AiHeapTypeObject *)type;
    XDEC_REFCNT(type->tp_base);
    XDEC_REFCNT(type->tp_dict);
    XDEC_REFCNT(type->tp_bases);
    XDEC_REFCNT(type->tp_mro);
    XDEC_REFCNT(type->tp_cache);
    XDEC_REFCNT(type->tp_subclasses);
    XDEC_REFCNT(et->ht_name);
    XDEC_REFCNT(et->ht_slots);
    OB_FREE(type);
}

void type_print(AiTypeObject *ob, FILE *stream) {
    fputs("<type 'type'>", stream);
}

AiObject *type_str(AiTypeObject *ob) {
    return CHECK_TYPE(ob, &type_typeobject) ? string_from_cstring("<type 'type'>") : OB_TO_STRING(ob);
}
