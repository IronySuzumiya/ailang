#include "../ailang.h"

static void iter_dealloc(AiIterObject *iter);
static AiObject *iter_iter(AiIterObject *iter);

AiTypeObject type_iterobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "iterator",                         /* tp_name */
    sizeof(AiIterObject),               /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)iter_dealloc,           /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//object_generic_getattr,             /* tp_getattro */
    0,                                  /* tp_setattro */

    0,                                  /* tp_flags */

    (unaryfunc)iter_iter,               /* tp_iter */
    (unaryfunc)iter_iternext,           /* tp_iternext */
    0,//iter_methods,                       /* tp_methods */
};

AiObject *iter_new(AiObject *seq) {
    AiIterObject *it;

    if (!CHECK_ITERABLE(seq)) {
        RUNTIME_EXCEPTION("object cannot be iterated");
        return NULL;
    }
    it = AiObject_GC_NEW(AiIterObject);
    INIT_AiObject(it, &type_iterobject);
    it->it_index = 0;
    INC_REFCNT(seq);
    it->it_seq = (AiListObject *)seq;

    return (AiObject *)it;
}

AiObject *iter_iternext(AiIterObject *iter) {
    if (!iter->it_seq) {
        return NULL;
    }
    if (iter->it_index < SEQUENCE_SIZE(iter->it_seq)) {
        AiObject *r = SEQUENCE_GETITEM(iter->it_seq, iter->it_index);
        ++iter->it_index;
        return r;
    }
    else {
        return NULL;
    }
}

void iter_dealloc(AiIterObject *iter) {
    XDEC_REFCNT(iter->it_seq);
    AiMEM_FREE(iter);
}

AiObject *iter_iter(AiIterObject *iter) {
    INC_REFCNT(iter);
    return (AiObject *)iter;
}
