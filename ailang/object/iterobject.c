#include "../ailang.h"

static void iter_dealloc(AiSeqiterObject *iter);
static AiObject *iter_iter(AiSeqiterObject *iter);
static AiObject *seqiter_iternext(AiSeqiterObject *iter);

AiTypeObject AiType_Seqiter = {
    INIT_AiVarObject_HEAD(&AiType_Type, 0)
    "iterator",                         /* tp_name */
    sizeof(AiSeqiterObject),               /* tp_basicsize */
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
    0,//AiObject_Generic_Getattr,             /* tp_getattro */
    0,                                  /* tp_setattro */

    0,                                  /* tp_flags */

    (unaryfunc)iter_iter,               /* tp_iter */
    (unaryfunc)seqiter_iternext,           /* tp_iternext */
    0,//iter_methods,                       /* tp_methods */
};

AiObject *AiSeqiter_New(AiObject *seq) {
    AiSeqiterObject *it;

    if (!CHECK_ITERABLE(seq)) {
        RUNTIME_EXCEPTION("object cannot be iterated");
        return NULL;
    }
    it = AiObject_GC_New(AiSeqiterObject);
    INIT_AiObject(it, &AiType_Seqiter);
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
