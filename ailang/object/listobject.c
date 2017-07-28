#include "../ailang.h"

static Object *list_getitem(ListObject *ob, ssize_t index) {
    if (CHECK_TYPE_LIST(ob)) {
        if (index < 0) {
            index += ob->ob_size;
        }
        if (index >= 0 && index < ob->ob_size) {
            return ob->ob_item[index];
        }
        else {
            RUNTIME_EXCEPTION("index out of range");
            return (Object *)none;
        }
    }
    else {
        return ob->ob_type->tp_as_sequence->sq_getitem((Object *)ob, index);
    }
}

TypeObject type_listobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)

};
ListObject *free_lists[NUMBER_FREE_LISTS_MAX];
int number_free_lists;

Object * list_new(ssize_t size) {
    ListObject *ob;
    size_t nbytes = (size_t)(size * sizeof(Object *));
    if (number_free_lists) {
        ob = free_lists[--number_free_lists];
        INIT_REFCNT(ob);
    }
    else {
        ob = gc_malloc(sizeof(ListObject));
        INIT_OBJECT_VAR(ob, &type_listobject, 0);
    }
    if (size <= 0) {
        ob->ob_item = NULL;
    }
    else {
        ob->ob_item = gc_malloc(nbytes);
        memset(ob->ob_item, 0, nbytes);
    }
    ob->ob_size = size;
    ob->allocated = size;
    return (Object *)ob;
}
