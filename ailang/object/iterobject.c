#include "../ailang.h"

static void iter_dealloc(AiIterObject *iter);
static AiObject *iter_iter(AiIterObject *iter);

AiTypeObject type_iterobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "iterator",
    (destructor)iter_dealloc,
    0,
    0,

    0,
    0,
    0,

    0,
    0,
    0,

    (unaryfunc)iter_iter,
    (unaryfunc)iter_iternext,
};

AiObject *iter_new(AiObject *seq) {
    AiIterObject *it;

    if (!CHECK_ITERABLE(seq)) {
        RUNTIME_EXCEPTION("object cannot be iterated");
        return NULL;
    }
    it = AiObject_GC_NEW(AiIterObject);
    INIT_OBJECT(it, &type_iterobject);
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
