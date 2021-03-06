#include "../ailang.h"

static void iter_dealloc(AiSeqiterObject *iter);
static AiObject *iter_iter(AiSeqiterObject *iter);
static AiObject *seqiter_iternext(AiSeqiterObject *iter);

AiTypeObject AiType_Seqiter = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "iterator",                         /* tp_name */
    sizeof(AiSeqiterObject),               /* tp_basicsize */
    0,                                  /* tp_itemsize */

    (destructor)iter_dealloc,           /* tp_dealloc */
    0,                                  /* tp_compare */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    (unaryfunc)iter_iter,               /* tp_iter */
    (unaryfunc)seqiter_iternext,           /* tp_iternext */
};

AiObject *AiSeqiter_New(AiObject *seq) {
    AiSeqiterObject *it;

    if (!CHECK_ITERABLE(seq)) {
        RUNTIME_EXCEPTION("object cannot be iterated");
        return NULL;
    }
    it = AiObject_NEW(AiSeqiterObject, &AiType_Seqiter);
    it->it_index = 0;
    INC_REFCNT(seq);
    it->it_seq = (AiListObject *)seq;

    return (AiObject *)it;
}

AiObject *seqiter_iternext(AiSeqiterObject *iter) {
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

void iter_dealloc(AiSeqiterObject *iter) {
    XDEC_REFCNT(iter->it_seq);
    AiMem_Free(iter);
}

AiObject *iter_iter(AiSeqiterObject *iter) {
    INC_REFCNT(iter);
    return (AiObject *)iter;
}
