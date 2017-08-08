#include "../ailang.h"

static void type_dealloc(AiTypeObject *type);
static AiObject *type_call(AiTypeObject *type, AiObject *args, AiObject *kwds);

AiTypeObject AiType_Type = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "type",                             /* tp_name */
    sizeof(AiTypeObject),               /* tp_basicsize */
    0,                                  /* tp_itemsize */

    (destructor)type_dealloc,           /* tp_dealloc */
    0,                                  /* tp_compare */
    (hashfunc)Pointer_Hash,             /* tp_hash */
    (ternaryfunc)type_call,             /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_new */
    0,                                  /* tp_init */
    AiObject_Del,                       /* tp_free */
};

int AiType_Ready(AiTypeObject *type) {
    if (!type->tp_base) {
        type->tp_base = &AiType_BaseObject;
    }
    INC_REFCNT(type->tp_base);
    if (!type->tp_dict) {
        type->tp_dict = AiDict_New();
    }
    inherit(type, type->tp_base);
    if (!type->tp_as_number) {
        type->tp_as_number = type->tp_base->tp_as_number;
    }
    if (!type->tp_as_sequence) {
        type->tp_as_sequence = type->tp_base->tp_as_sequence;
    }
    if (!type->tp_as_mapping) {
        type->tp_as_mapping = type->tp_base->tp_as_mapping;
    }
    return 0;
}

AiObject *AiType_Generic_Alloc(AiTypeObject *type, ssize_t nitems) {
    AiObject *obj;
    ssize_t size = _AiVarObject_SIZE(type, nitems + 1);

    obj = (AiObject *)AiObject_Malloc(size);
    AiMem_Set(obj, 0, size);
    INC_REFCNT(type);
    if (type->tp_itemsize) {
        AiVarObject_INIT(obj, type, nitems);
    }
    else {
        AiObject_INIT(obj, type);
    }
    return obj;
}

void type_dealloc(AiTypeObject *type) {
    DEC_REFCNT(type->tp_dict);
    DEC_REFCNT(type->tp_base);
    OB_FREE(type);
}

AiObject *type_call(AiTypeObject *type, AiObject *args, AiObject *kwds) {
    
}
