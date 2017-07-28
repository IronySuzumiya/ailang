#define CRTDBG_MAP_ALLOC

#include "../ailang.h"
#include <crtdbg.h>

int main() {
    int_init();
    Object *v1 = int_fromlong(998);
    Object *v2 = int_fromlong(-33);
    Object *v3 = v1->ob_type->tp_as_number->nb_divide(v1, v2);
    PRINT_STDOUT(v1);
    PRINT_STDOUT(v2);
    PRINT_STDOUT(v3);
    DEC_REFCNT(v2);
    v2 = int_fromlong(-56);
    PRINT_STDOUT(v2);
    DEC_REFCNT(v1);
    DEC_REFCNT(v2);
    DEC_REFCNT(v3);

    Object *s1 = string_fromcstring("Hello World");
    PRINT_STDOUT(s1);
    DEC_REFCNT(s1);

    IntBlock *s = block_list;
    IntBlock *n = s;
    while (s) {
        n = s->next;
        gc_free(s);
        s = n;
    }

    _CrtDumpMemoryLeaks();
    return 0;
}