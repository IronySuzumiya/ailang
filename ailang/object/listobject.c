#include "../ailang.h"

static int list_resize(AiListObject *list, ssize_t newsize);
static void list_dealloc(AiListObject *list);
static int list_compare(AiListObject *lhs, AiListObject *rhs);
static ssize_t list_size(AiListObject *list);
static AiObject *list_slice(AiListObject *list, ssize_t start, ssize_t end);
static int list_contains(AiListObject *list, AiObject *item);
static AiObject *list_str(AiListObject *list);
static int list_insert(AiListObject *list, ssize_t index, AiObject *item);
static int list_extend(AiListObject *fo, AiListObject *la);
static AiObject *list_iter(AiObject *seq);

static AiListObject *free_lists[NUMBER_FREE_LISTS_MAX];
static int number_free_lists;

static AiSequenceMethods list_as_sequence = {
    (lengthfunc)list_size,
    (binaryfunc)list_extend,
    (ssizeargfunc)AiList_GetItem,
    (sqsetitemfunc)AiList_SetItem,
    (ssize2argfunc)list_slice,
    (enquiry2)list_contains,
};

AiTypeObject AiType_List = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "list",                             /* tp_name */
    sizeof(AiListObject),               /* tp_basesize */
    0,                                  /* tp_itemsize */

    (destructor)list_dealloc,           /* tp_dealloc */
    (cmpfunc)list_compare,              /* tp_compare */
    (hashfunc)AiObject_Unhashable,      /* tp_hash */
    0,                                  /* tp_call */
    (unaryfunc)list_str,                /* tp_str */
    list_iter,                          /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    &list_as_sequence,                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//type_generic_new                    /* tp_new */
    0,//(initproc)list_init,                /* tp_init */
    AiObject_Del,                       /* tp_free */
};

AiObject * AiList_New(ssize_t size) {
    AiListObject *list;
    ssize_t nbytes = size * sizeof(AiObject *);
    if (number_free_lists) {
        list = free_lists[--number_free_lists];
        INIT_REFCNT(list);
    }
    else {
        list = AiVarObject_NEW(AiListObject, &AiType_List, 0);
    }
    if (size <= 0) {
        list->ob_item = NULL;
    }
    else {
        list->ob_item = AiMem_Alloc(nbytes);
        AiMem_Set(list->ob_item, 0, nbytes);
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
    list->ob_item = list->ob_item ?
        AiMem_Realloc(list->ob_item, newsize * sizeof(AiObject *)) :
        AiMem_Alloc(newsize * sizeof(AiObject *));
    LIST_SIZE(list) = newsize;
    list->allocated = new_allo;
    return 0;
}

AiObject *AiList_GetItem(AiListObject *list, ssize_t index) {
    MAKE_INDEX_IN_RANGE(index, LIST_SIZE(list));
    if (index < LIST_SIZE(list)) {
        return list->ob_item[index];
    }
    else {
        RUNTIME_EXCEPTION("index out of range");
        return NULL;
    }
}

int AiList_SetItem(AiListObject *list, ssize_t index, AiObject *newitem) {
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
            newlist = (AiListObject *)AiList_New(end - start);
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
        if (AiObject_Rich_Compare(list->ob_item[i], item, CMP_EQ) > 0) {
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
        return AiString_From_String("[]");
    }
    else if (LIST_SIZE(list) == 1) {
        item = (AiStringObject *)OB_TO_STRING(list->ob_item[0]);
        str = (AiStringObject *)AiString_From_StringAndSize(NULL, STRING_LEN(item) + 2);
        STRING_AS_CSTRING(str)[0] = '[';
        AiMem_Copy(&STRING_AS_CSTRING(str)[1], STRING_AS_CSTRING(item), STRING_LEN(item));
        STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = ']';
        DEC_REFCNT(item);
        return (AiObject *)str;
    }

    strlist = (AiListObject *)AiList_New(LIST_SIZE(list));

    item = (AiStringObject *)OB_TO_STRING(list->ob_item[0]);
    size = STRING_LEN(item);
    strlist->ob_item[0] = AiString_From_StringAndSize(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[0])[0] = '[';
    AiMem_Copy(&STRING_AS_CSTRING(strlist->ob_item[0])[1], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    item = (AiStringObject *)OB_TO_STRING(list->ob_item[LIST_SIZE(list) - 1]);
    size = STRING_LEN(item);
    strlist->ob_item[LIST_SIZE(list) - 1] = AiString_From_StringAndSize(NULL, size + 1);
    STRING_AS_CSTRING(strlist->ob_item[LIST_SIZE(list) - 1])[size] = ']';
    AiMem_Copy(&STRING_AS_CSTRING(strlist->ob_item[LIST_SIZE(list) - 1])[0], STRING_AS_CSTRING(item), size);
    DEC_REFCNT(item);

    for (ssize_t i = 1; i < LIST_SIZE(list) - 1; ++i) {
        strlist->ob_item[i] = OB_TO_STRING(list->ob_item[i]);
    }
    split = (AiStringObject *)AiString_From_String(", ");
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
    if (CHECK_EXACT_TYPE_LIST(fo) && CHECK_EXACT_TYPE_LIST(la)) {
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

int AiList_ClearAllMemory() {
    while (number_free_lists--) {
        AiMem_Free(free_lists[number_free_lists]);
    }
    return 0;
}

ssize_t list_size(AiListObject *list) {
    return CHECK_EXACT_TYPE_LIST(list) ? LIST_SIZE(list) : list->ob_type->tp_as_sequence->sq_length((AiObject *)list);
}

void list_dealloc(AiListObject *list) {
    if (list->ob_item) {
        for (ssize_t i = 0; i < LIST_SIZE(list); ++i) {
            XDEC_REFCNT(list->ob_item[i]);
        }
        AiMem_Free(list->ob_item);
    }
    if (CHECK_EXACT_TYPE_LIST(list) && number_free_lists < NUMBER_FREE_LISTS_MAX) {
        free_lists[number_free_lists++] = list;
    }
    else {
        OB_FREE(list);
    }
}

int list_compare(AiListObject *lhs, AiListObject *rhs) {
    ssize_t minsize = min(LIST_SIZE(lhs), LIST_SIZE(rhs));
    int r;

    for (ssize_t i = 0; i < minsize; ++i) {
        r = AiObject_Rich_Compare(lhs->ob_item[i], rhs->ob_item[i], CMP_NE);
        if (r > 0) {
            return AiObject_Rich_Compare(lhs->ob_item[i], rhs->ob_item[i], CMP_GT) > 0 ? 1 : -1;
        }
        else if (r < 0) {
            RUNTIME_EXCEPTION("there's no compare method between '%s' and '%s' yet", OB_TYPENAME(lhs->ob_item[i]), OB_TYPENAME(rhs->ob_item[i]));
        }
    }
    return LIST_SIZE(lhs) > LIST_SIZE(rhs) ? 1 : LIST_SIZE(lhs) < LIST_SIZE(rhs) ? -1 : 0;
}

AiObject *list_iter(AiObject *seq) {
    AiSeqiterObject *it;
    it = AiObject_NEW(AiSeqiterObject, &AiType_Seqiter);
    it->it_index = 0;
    INC_REFCNT(seq);
    it->it_seq = (AiListObject *)seq;
    return (AiObject *)it;
}
