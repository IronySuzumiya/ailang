#define CRTDBG_MAP_ALLOC

#include "../ailang.h"
#include <crtdbg.h>

int main() {
    AiThreadState *t = threadstate_new(interpreterstate_new());
    threadstate_swap(t);

    int_init();

    AiObject *v1 = int_from_long(998);
    AiObject *v2 = int_from_long(-33);
    AiObject *v3 = v1->ob_type->tp_as_number->nb_divide(v1, v2);
    OB_PRINT_STDOUT(v1);
    OB_PRINT_STDOUT(v2);
    OB_PRINT_STDOUT(v3);
    DEC_REFCNT(v2);
    v2 = int_from_long(-56);
    OB_PRINT_STDOUT(v2);

    AiObject *s1 = string_from_cstring("Hello World");
    OB_PRINT_STDOUT(s1);

    AiObject *s2 = v1->ob_type->tp_as_number->nb_add(v1, s1);
    OB_PRINT_STDOUT(t->curexc_value);
    exception_clear();

    AiListObject *list = (AiListObject *)list_new(4);

    list_setitem(list, 0, v1);
    list_setitem(list, 1, v2);
    list_setitem(list, 2, v3);
    list_setitem(list, 3, s1);

    AiStringObject *liststr = (AiStringObject *)list_to_string(list);

    OB_PRINT_STDOUT(liststr);

    AiDictObject *dict = (AiDictObject *)dict_new();

    dict_setitem(dict, s1, v1);
    dict_setitem(dict, v2, v3);

    AiStringObject *dictstr = (AiStringObject *)dict_to_string(dict);

    OB_PRINT_STDOUT(dictstr);

    DEC_REFCNT(dictstr);
    DEC_REFCNT(dict);
    DEC_REFCNT(liststr);
    DEC_REFCNT(list);
    DEC_REFCNT(v1);
    DEC_REFCNT(v2);
    DEC_REFCNT(v3);
    DEC_REFCNT(s1);

    int_clear_blocks();
    list_clear_free_lists();
    dict_clear_free_dicts_and_dummy();

    t = threadstate_swap(NULL);
    interpreterstate_delete(t->interp);

    assert(heaphead == NULL);

    _CrtDumpMemoryLeaks();
    return 0;
}