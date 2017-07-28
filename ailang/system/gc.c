#include "../ailang.h"

void *gc_malloc(ssize_t size) {
    void *m = malloc((size_t)size);
    if (m) {
        return m;
    }
    else {
        FATAL_ERROR("bad malloc");
        return NULL;
    }
}

void gc_free(void *p) {
    free(p);
}
