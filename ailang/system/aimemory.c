#include "../ailang.h"

void *mem_alloc(ssize_t size) {
    void *m = malloc((size_t)size);
    if (m) {
        return m;
    }
    else {
        FATAL_ERROR("bad malloc");
        return NULL;
    }
}

void mem_free(void *p) {
    free(p);
}

void *mem_realloc(void *p, ssize_t size) {
    return realloc(p, (size_t)size);
}

void mem_copy(void *dst, void *src, ssize_t size) {
    memcpy(dst, src, (size_t)size);
}

void mem_set(void *p, int val, ssize_t size) {
    memset(p, val, (size_t)size);
}
