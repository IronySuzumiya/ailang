#include "../ailang.h"

static AiInterpreterState *interp_head;

AiThreadState *AiThreadState_Current;

AiInterpreterState *AiInterpreterState_New() {
    AiInterpreterState *interp = (AiInterpreterState *)AiMem_Alloc(sizeof(AiInterpreterState));

    interp->modules = NULL;
    interp->sysdict = NULL;
    interp->builtins = NULL;
    interp->tstate_head = NULL;

    interp->next = interp_head;
    interp_head = interp;

    return interp;
}

void AiInterpreterState_Clear(AiInterpreterState *interp) {
    AiThreadState *p = interp->tstate_head;
    while (p) {
        AiThreadState_Clear(p);
        p = p->next;
    }
    OB_CLEAR(interp->modules);
    OB_CLEAR(interp->sysdict);
    OB_CLEAR(interp->builtins);
}

void AiInterpreterState_Delete(AiInterpreterState *interp) {
    AiInterpreterState **p;
    AiThreadState *t;
    while (t = interp->tstate_head) {
        AiThreadState_Delete(t);
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
    AiMem_Free(interp);
}

AiThreadState *AiThreadState_New(AiInterpreterState *interp) {
    AiThreadState *tstate = (AiThreadState *)AiMem_Alloc(sizeof(AiThreadState));

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

void AiThreadState_Clear(AiThreadState *tstate) {
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

void AiThreadState_Delete(AiThreadState *tstate) {
    AiInterpreterState *interp;
    AiThreadState **p;
    AiThreadState *prev_p = NULL;

    if (tstate == AiThreadState_Current)
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
    AiMem_Free(tstate);
}

AiThreadState *AiThreadState_Get() {
    if (!AiThreadState_Current)
        FATAL_ERROR("PyThreadState_Get: no current thread");
    return AiThreadState_Current;
}


AiThreadState *AiThreadState_Swap(AiThreadState *newts) {
    AiThreadState *oldts = AiThreadState_Current;
    AiThreadState_Current = newts;
    return oldts;
}

AiObject *AiThreadState_Getdict() {
    if (!AiThreadState_Current)
        return NULL;
    if (!AiThreadState_Current->dict) {
        AiThreadState_Current->dict = AiDict_New();
    }
    return AiThreadState_Current->dict;
}

AiObject *AiThreadState_Current_Frame() {
    AiObject *result = NULL;
    AiInterpreterState *i;

    result = AiDict_New();

    for (i = interp_head; i; i = i->next) {
        AiThreadState *t;
        for (t = i->tstate_head; t; t = t->next) {
            AiObject *id;
            AiObject *frame = (AiObject *)t->frame;
            if (frame) {
                id = AiInt_From_Long(t->thread_id);
                AiDict_SetItem((AiDictObject *)result, id, frame);
                DEC_REFCNT(id);
            }
        }
    }
    return result;
}
