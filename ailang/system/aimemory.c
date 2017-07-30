#include "../ailang.h"

HeapList *heaphead;
static HeapList *heapptr;

void *mem_alloc(ssize_t size) {
    void *m = malloc((size_t)size);
    if (m) {
#ifdef DEBUG
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
#ifdef DEBUG
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
        return NULL;
    }
    if (!prev) {
        heaphead = hp->next;
    }
    else {
        prev->next = hp->next;
    }
    free(hp);
#endif
    free(p);
}

void *mem_realloc(void *p, ssize_t size) {
#ifdef DEBUG
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
#ifdef DEBUG
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
