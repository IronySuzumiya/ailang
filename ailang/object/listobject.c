#include "../ailang.h"

static void list_dealloc(AiListObject *list);
static void list_print(AiListObject *list, FILE *stream);
static int list_compare(AiListObject *lhs, AiListObject *rhs);
static ssize_t list_size(AiListObject *list);

static AiListObject *free_lists[NUMBER_FREE_LISTS_MAX];
static int number_free_lists;

static AiSequenceMethods list_as_sequence = {
    (lengthfunc)list_size,
    (binaryfunc)list_extend,
    (ssizeargfunc)list_getitem,
    (sqsetitemfunc)list_setitem,
    (ssize2argfunc)list_slice,
    (enquiry2)list_contains,
};

AiTypeObject type_listobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "list",                             /* tp_name */
    sizeof(AiListObject),               /* tp_basesize */
    0,                                  /* tp_itemsize */
    (destructor)list_dealloc,           /* tp_dealloc */
    (printfunc)list_print,              /* tp_print */
    (cmpfunc)list_compare,              /* tp_compare */

    0,                                  /* tp_as_number */
    &list_as_sequence,                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,//(hashfunc)object_unhashable,        /* tp_hash */
    0,                                  /* tp_call */
    (unaryfunc)list_str,                /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//object_generic_getattr,             /* tp_getattro */
    0,                                  /* tp_setattro */

    SUBCLASS_LIST | BASE_TYPE,          /* tp_flags */

    0,//list_iter,                          /* tp_iter */
    0,                                  /* tp_iternext */

    0,//list_methods,                       /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,//(initproc)list_init,                /* tp_init */
    0,//type_generic_alloc,                 /* tp_alloc */
    0,//type_generic_new                    /* tp_new */
    AiObject_GC_DEL,                    /* tp_free */
};

AiObject * list_new(ssize_t size) {
    AiListObject *list;
    ssize_t nbytes = size * sizeof(AiObject *);
    if (number_free_lists) {
        list = free_lists[--number_free_lists];
        INIT_REFCNT(list);
    }
    else {
        list = AiObject_GC_NEW(AiListObject);
        INIT_OBJECT_VAR(list, &type_listobject, 0);
    }
    if (size <= 0) {
        list->ob_item = NULL;
    }
    else {
        list->ob_item = AiMEM_ALLOC(nbytes);
        AiMEM_SET(list->ob_item, 0, nbytes);
    }
    LIST_SIZE(list) = size;
    list->allocated = size;
    return (AiObject *)list;
}

int list_resize(AiListObject *list, ssize_t newsize) {
    ssize_t old_allo = list->allocated, new_allo;
    if (newsize < old_allo && newsize >= old_allo / 2) {
        return 0;
    }
    // The growth pattern is:  0, 4, 8, 16, 25, 35, 46, 58, 72, 88, ...
    new_allo = newsize + (newsize >> 3) + (newsize < 9 ? 3 : 6);
    list->ob_item = AiMEM_REALLOC(list->ob_item, newsize);
    LIST_SIZE(list) = newsize;
    list->allocated = new_allo;
    return 0;
}

AiObject *list_getitem(AiListObject *list, ssize_t index) {
    MAKE_INDEX_IN_RANGE(index, LIST_SIZE(list));
    if (index < LIST_SIZE(list)) {
        return list->ob_item[index];
    }
    else {
        RUNTIME_EXCEPTION("index out of range");
        return NULL;
    }
}

int list_setitem(AiListObject *list, ssize_t index, AiObject *newitem) {
    MAKE_INDEX_IN_RANGE(index, LIST_SIZE(list));
    if (index < LIST_SIZE(list)) {
        XDEC_REFCNT(list->ob_item[index]);
        list->ob_item[index] = newitem;
        XINC_REFCNT(newitem);
        return 0;
    }
    else {
        RUNTIME_EXCEPTION("index out of range");
        return -1;
    }
}

AiObject *list_slice(AiListObject *list, ssize_t start, ssize_t end) {
    AiListObject *newlist;
    MAKE_INDEX_IN_RANGE(start, LIST_SIZE(list));
    MAKE_INDEX_IN_RANGE(end, LIST_SIZE(list));
    if (start < LIST_SIZE(list) && start < end) {
        if (end > LIST_SIZE(list)) {
            end = LIST_SIZE(list);
        }
        if (start == 0 && end == LIST_SIZE(list)) {
            INC_REFCNT(list);
            return (AiObject *)list;
        }
        else {
            newlist = (AiListObject *)list_new(end - start);
            for (ssize_t i = 0; i < end - start; ++i) {
                newlist->ob_item[i] = list->ob_item[start + i];
                XINC_REFCNT(newlist->ob_item[i]);
            }
            return (AiObject *)newlist;
        }
    }
    else {
        RUNTIME_EXCEPTION("invalid range sliced");
        return NULL;
    }
}

int list_contains(AiListObject *list, AiObject *item) {
    for (ssize_t i = 0; i < LIST_SIZE(list); ++i) {
        if (object_rich_compare(list->ob_item[i], item, CMP_EQ) > 0) {
            return 1;
        }
    }
    return 0;
}

AiObject *list_str(AiListObject *list) {
    AiListObject *strlist;
    AiStringObject *str;
    AiStringObject *item;
    AiStringObject *split;
    ssize_t size;

    if (LIST_SIZE(list) == 0) {
        return string_from_cstring("[]");
    }
    else if (LIST_SIZE(list) == 1) {
        item = (AiStringObject *)OB_TO_STRING(list->ob_item[0]);
        str = (AiStringObject *)string_from_cstring_with_size(NULL, STRING_LEN(item) + 2);
        STRING_AS_CSTRING(str)[0] = '[';
        AiMEM_COPY(&STRING_AS_CSTRING(str)[1], STRING_AS_CSTRING(item), STRING_LEN(item));
        STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = ']';
        DEC_REFCNT(item);
        return (AiObject *)str;
    }

    strlist = (AiListObject *)list_new(LIST_SIZE(list));

    item = (AiStringObject *)OB_TO_STRING(list->ob_item[0]);
    size = STRING_LEN(item);
    strlist->ob_item[0] = string_from_cstring_with_size(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[0])[0] = '[';
    AiMEM_COPY(&STRING_AS_CSTRING(strlist->ob_item[0])[1], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    item = (AiStringObject *)OB_TO_STRING(list->ob_item[LIST_SIZE(list) - 1]);
    size = STRING_LEN(item);
    strlist->ob_item[LIST_SIZE(list) - 1] = string_from_cstring_with_size(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[LIST_SIZE(list) - 1])[size] = ']';
    AiMEM_COPY(&STRING_AS_CSTRING(strlist->ob_item[LIST_SIZE(list) - 1])[0], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    for (ssize_t i = 1; i < LIST_SIZE(list) - 1; ++i) {
        strlist->ob_item[i] = OB_TO_STRING(list->ob_item[i]);
    }
    split = (AiStringObject *)string_from_cstring(", ");
    str = (AiStringObject *)string_join(split, (AiObject *)strlist);

    split->ob_type->tp_dealloc((AiObject *)split);
    strlist->ob_type->tp_dealloc((AiObject *)strlist);

    return (AiObject *)str;
}

int list_insert(AiListObject *list, ssize_t index, AiObject *item) {
    MAKE_INDEX_IN_RANGE(index, LIST_SIZE(list));
    if (index < LIST_SIZE(list)) {
        list_resize(list, LIST_SIZE(list) + 1);
        for (ssize_t i = index + 1; i < LIST_SIZE(list); ++i) {
            list->ob_item[i] = list->ob_item[i - 1];
        }
        list->ob_item[index] = item;
        XINC_REFCNT(item);
        return 0;
    }
    else {
        RUNTIME_EXCEPTION("index out of range");
        return -1;
    }
}

int list_append(AiListObject *list, AiObject *item) {
    list_resize(list, LIST_SIZE(list) + 1);
    list->ob_item[LIST_SIZE(list) - 1] = item;
    XINC_REFCNT(item);
    return 0;
}

int list_extend(AiListObject *fo, AiListObject *la) {
    if (CHECK_TYPE_LIST(fo) && CHECK_TYPE_LIST(la)) {
        ssize_t fo_size = LIST_SIZE(fo), la_size = LIST_SIZE(la);
        list_resize(fo, fo_size + la_size);
        for (ssize_t i = 0; i < la_size; ++i) {
            fo->ob_item[fo_size + i] = la->ob_item[i];
            XINC_REFCNT(la->ob_item[i]);
        }
        return 0;
    }
    else {
        UNSUPPORTED_EXTEND(OB_TYPENAME(fo), OB_TYPENAME(la));
        return -1;
    }
}

int list_clear_free_lists() {
    while (number_free_lists--) {
        AiMEM_FREE(free_lists[number_free_lists]);
    }
    return 0;
}

ssize_t list_size(AiListObject *list) {
    return CHECK_TYPE_LIST(list) ? LIST_SIZE(list) : list->ob_type->tp_as_sequence->sq_length((AiObject *)list);
}

void list_dealloc(AiListObject *list) {
    if (list->ob_item) {
        for (ssize_t i = 0; i < LIST_SIZE(list); ++i) {
            XDEC_REFCNT(list->ob_item[i]);
        }
        AiMEM_FREE(list->ob_item);
    }
    if (CHECK_TYPE_LIST(list) && number_free_lists < NUMBER_FREE_LISTS_MAX) {
        free_lists[number_free_lists++] = list;
    }
    else {
        OB_FREE(list);
    }
}

void list_print(AiListObject *list, FILE *stream) {
    fputs("[", stream);
    if (list->ob_item && LIST_SIZE(list) > 0) {
        for (ssize_t i = 0; i < LIST_SIZE(list) - 1; ++i) {
            if (!list->ob_item[i])
                continue;
            OB_PRINT(list->ob_item[i], stream);
            fputs(", ", stream);
        }
        OB_PRINT(list->ob_item[LIST_SIZE(list) - 1], stream);
    }
    fputs("]", stream);
}

int list_compare(AiListObject *lhs, AiListObject *rhs) {
    ssize_t minsize = min(LIST_SIZE(lhs), LIST_SIZE(rhs));
    int r;

    for (ssize_t i = 0; i < minsize; ++i) {
        r = object_rich_compare(lhs->ob_item[i], rhs->ob_item[i], CMP_NE);
        if (r > 0) {
            return object_rich_compare(lhs->ob_item[i], rhs->ob_item[i], CMP_GT) > 0 ? 1 : -1;
        }
        else if (r < 0) {
            RUNTIME_EXCEPTION("there's no compare method between '%s' and '%s' yet", OB_TYPENAME(lhs->ob_item[i]), OB_TYPENAME(rhs->ob_item[i]));
        }
    }
    return LIST_SIZE(lhs) > LIST_SIZE(rhs) ? 1 : LIST_SIZE(lhs) < LIST_SIZE(rhs) ? -1 : 0;
}
