#define CRTDBG_MAP_ALLOC

#include "../ailang.h"
#include <crtdbg.h>

int main() {
    int_init();
    AiObject *v1 = int_from_long(998);
    AiObject *v2 = int_from_long(-33);
    AiObject *v3 = v1->ob_type->tp_as_number->nb_divide(v1, v2);
    PRINT_STDOUT(v1);
    PRINT_STDOUT(v2);
    PRINT_STDOUT(v3);
    DEC_REFCNT(v2);
    v2 = int_from_long(-56);
    PRINT_STDOUT(v2);

    AiObject *s1 = string_from_cstring("Hello World");
    PRINT_STDOUT(s1);

    AiObject *s2 = v1->ob_type->tp_as_number->nb_add(v1, s1);

    AiIntBlock *s = block_list;
    AiIntBlock *n = s;

    AiListObject *list = (AiListObject *)list_new(4);

    list_setitem(list, 0, v1);
    list_setitem(list, 1, v2);
    list_setitem(list, 2, v3);
    list_setitem(list, 3, s1);

    AiStringObject *liststr = (AiStringObject *)list_to_string(list);

    PRINT_STDOUT(liststr);

    DEC_REFCNT(liststr);
    DEC_REFCNT(list);
    DEC_REFCNT(v1);
    DEC_REFCNT(v2);
    DEC_REFCNT(v3);
    DEC_REFCNT(s1);

    while (s) {
        n = s->next;
        AiMEM_FREE(s);
        s = n;
    }

    while (number_free_lists--) {
        AiMEM_FREE(free_lists[number_free_lists]);
    }

    _CrtDumpMemoryLeaks();
    return 0;
}