#include "../ailang.h"

AiTypeObject AiType_Traceback = {
    INIT_AiVarObject_HEAD(&AiType_Type, 0)
    "traceback",
};

void AiTraceback_Here(AiFrameObject *frame) {
    AiThreadState *tstate = frame->f_tstate;
    AiTracebackObject *oldtb = (AiTracebackObject *)tstate->curexc_traceback;
    AiTracebackObject *tb = AiTraceback_New(oldtb, frame);
    tstate->curexc_traceback = (AiObject *)tb;
    XDEC_REFCNT(oldtb);
}

AiTracebackObject *AiTraceback_New(AiTracebackObject *next, AiFrameObject *frame) {
    AiTracebackObject *tb;
    tb = AiObject_GC_New(AiTracebackObject);
    INIT_AiObject(tb, &AiType_Traceback);
    tb->tb_next = next;
    tb->tb_frame = frame;
    tb->tb_lasti = frame->f_lasti;
    tb->tb_lineno = AiCode_Addr2Line(frame->f_code, frame->f_lasti);
    return tb;
}
