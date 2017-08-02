#include "../ailang.h"

static AiInterpreterState *interp_head;

AiThreadState *threadstate_current;

AiInterpreterState *interpreterstate_new() {
    AiInterpreterState *interp = (AiInterpreterState *)AiMEM_ALLOC(sizeof(AiInterpreterState));

    interp->modules = NULL;
    interp->sysdict = NULL;
    interp->builtins = NULL;
    interp->tstate_head = NULL;

    interp->next = interp_head;
    interp_head = interp;

    return interp;
}

void interpreterstate_clear(AiInterpreterState *interp) {
    AiThreadState *p = interp->tstate_head;
    while (p) {
        threadstate_clear(p);
        p = p->next;
    }
    OB_CLEAR(interp->modules);
    OB_CLEAR(interp->sysdict);
    OB_CLEAR(interp->builtins);
}

void interpreterstate_delete(AiInterpreterState *interp) {
    AiInterpreterState **p;
    AiThreadState *t;
    while (t = interp->tstate_head) {
        threadstate_delete(t);
    }
    for (p = &interp_head; ; p = &(*p)->next) {
        if (!(*p)) {
            FATAL_ERROR("invalid interp");
            break;
        }
        if (*p == interp) {
            break;
        }
    }
    if (interp->tstate_head)
        FATAL_ERROR("remaining threads");
    *p = interp->next;
    AiMEM_FREE(interp);
}

AiThreadState *threadstate_new(AiInterpreterState *interp) {
    AiThreadState *tstate = (AiThreadState *)AiMEM_ALLOC(sizeof(AiThreadState));

    tstate->interp = interp;

    tstate->frame = NULL;
    tstate->recursion_depth = 0;
    tstate->thread_id = GetCurrentThreadId();

    tstate->dict = NULL;

    tstate->curexc_type = NULL;
    tstate->curexc_value = NULL;
    tstate->curexc_traceback = NULL;

    tstate->exc_type = NULL;
    tstate->exc_value = NULL;
    tstate->exc_traceback = NULL;

    tstate->next = interp->tstate_head;
    interp->tstate_head = tstate;

    return tstate;
}

void threadstate_clear(AiThreadState *tstate) {
    if (tstate->frame == NULL)
        fprintf(stderr, "warning: thread still has a frame\n");

    OB_CLEAR(tstate->frame);

    OB_CLEAR(tstate->dict);

    OB_CLEAR(tstate->curexc_type);
    OB_CLEAR(tstate->curexc_value);
    OB_CLEAR(tstate->curexc_traceback);

    OB_CLEAR(tstate->exc_type);
    OB_CLEAR(tstate->exc_value);
    OB_CLEAR(tstate->exc_traceback);
}

void threadstate_delete(AiThreadState *tstate) {
    AiInterpreterState *interp;
    AiThreadState **p;
    AiThreadState *prev_p = NULL;

    if (tstate == threadstate_current)
        FATAL_ERROR("tstate is still current");

    if (!tstate)
        FATAL_ERROR("NULL tstate");
    interp = tstate->interp;
    if (!interp)
        FATAL_ERROR("NULL interp");

    for (p = &interp->tstate_head; ; p = &(*p)->next) {
        if (!(*p))
            FATAL_ERROR("invalid tstate");
        if (*p == tstate)
            break;
        if (*p == prev_p)
            FATAL_ERROR("small circular list(!) and tstate not found.");
        prev_p = *p;
        if ((*p)->next == interp->tstate_head)
            FATAL_ERROR("circular list(!) and tstate not found.");
    }
    *p = tstate->next;
    AiMEM_FREE(tstate);
}

AiThreadState *threadstate_get() {
    if (!threadstate_current)
        FATAL_ERROR("PyThreadState_Get: no current thread");
    return threadstate_current;
}


AiThreadState *threadstate_swap(AiThreadState *newts) {
    AiThreadState *oldts = threadstate_current;
    threadstate_current = newts;
    return oldts;
}

AiObject *threadstate_getdict() {
    if (!threadstate_current)
        return NULL;
    if (!threadstate_current->dict) {
        threadstate_current->dict = dict_new();
    }
    return threadstate_current->dict;
}

AiObject *threadstate_current_frame() {
    AiObject *result = NULL;
    AiInterpreterState *i;

    result = dict_new();

    for (i = interp_head; i; i = i->next) {
        AiThreadState *t;
        for (t = i->tstate_head; t; t = t->next) {
            AiObject *id;
            AiObject *frame = (AiObject *)t->frame;
            if (frame) {
                id = int_from_long(t->thread_id);
                dict_setitem((AiDictObject *)result, id, frame);
                DEC_REFCNT(id);
            }
        }
    }
    return result;
}
