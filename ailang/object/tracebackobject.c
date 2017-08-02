#include "../ailang.h"

AiTypeObject type_tracebackobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "traceback",
};

void traceback_here(AiFrameObject *frame) {
    AiThreadState *tstate = frame->f_tstate;
    AiTracebackObject *oldtb = (AiTracebackObject *)tstate->curexc_traceback;
    AiTracebackObject *tb = traceback_new(oldtb, frame);
    tstate->curexc_traceback = (AiObject *)tb;
    XDEC_REFCNT(oldtb);
}

AiTracebackObject *traceback_new(AiTracebackObject *next, AiFrameObject *frame) {
    AiTracebackObject *tb;
    tb = AiObject_GC_NEW(AiTracebackObject);
    INIT_OBJECT(tb, &type_tracebackobject);
    tb->tb_next = next;
    tb->tb_frame = frame;
    tb->tb_lasti = frame->f_lasti;
    tb->tb_lineno = code_addr2line(frame->f_code, frame->f_lasti);
    return tb;
}
