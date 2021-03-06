#include "../ailang.h"

HeapList *heaphead;
static HeapList *heapptr;

void *mem_alloc(ssize_t size) {
    void *m = malloc((size_t)size);
    if (m) {
#ifdef AI_DEBUG
        if (!heaphead) {
            heapptr = malloc(sizeof(HeapList));
            heaphead = heapptr;
        }
        else {
            heapptr->next = malloc(sizeof(HeapList));
            heapptr = heapptr->next;
        }
        heapptr->mem = m;
        heapptr->next = NULL;
#endif
        return m;
    }
    else {
        FATAL_ERROR("bad malloc");
        return NULL;
    }
}

void mem_free(void *p) {
#ifdef AI_DEBUG
    HeapList *hp = heaphead;
    HeapList *prev = NULL;
    while (hp) {
        if (hp->mem == p) {
            break;
        }
        prev = hp;
        hp = hp->next;
    }
    if (!hp) {
        FATAL_ERROR("bad memory tracing");
        return;
    }
    if (!prev) {
        heaphead = hp->next;
    }
    else {
        prev->next = hp->next;
    }
    if (heapptr == hp) {
        heapptr = prev;
    }
    free(hp);
#endif
    free(p);
}

void *mem_realloc(void *p, ssize_t size) {
#ifdef AI_DEBUG
    HeapList *hp = heaphead;
    while (hp) {
        if (hp->mem == p) {
            break;
        }
        hp = hp->next;
    }
    if (!hp) {
        FATAL_ERROR("bad memory tracing");
        return NULL;
    }
#endif
    return
#ifdef AI_DEBUG
        hp->mem =
#endif
        realloc(p, (size_t)size);
}

void mem_copy(void *dst, void *src, ssize_t size) {
    memcpy(dst, src, (size_t)size);
}

void mem_set(void *p, int val, ssize_t size) {
    memset(p, val, (size_t)size);
}

AiObject *AiObject_Init(AiObject *ob, AiTypeObject *type) {
    OB_TYPE(ob) = type;
    INIT_REFCNT(ob);
    return ob;
}

AiVarObject *AiVarObject_Init(AiVarObject *ob, AiTypeObject *type, ssize_t size) {
    OB_SIZE(ob) = size;
    OB_TYPE(ob) = type;
    INIT_REFCNT(ob);
    return ob;
}
