#include "../ailang.h"

static AiObject *dict_str(AiDictObject *mp);
static ssize_t dict_size(AiDictObject *mp);
static int dict_resize(AiDictObject *mp, ssize_t minused);
static void dict_dealloc(AiDictObject *mp);
static void dict_print(AiDictObject *mp, FILE *stream);

static AiObject *dummy;
static AiDictEntry *freeslot;
static AiDictObject *free_dicts[NUMBER_FREE_DICTS_MAX];
static int number_free_dicts;

static AiMethodDef dict_methods[] = {
    { "__getitem__", (AiCFunction)AiDict_GetItem, METH_O },
    { NULL }
};

static AiMappingMethods dict_as_mapping = {
    (lengthfunc)dict_size,
    (binaryfunc)AiDict_GetItem,
    (mpsetitemfunc)AiDict_SetItem,
    (enquiry2)AiDict_DelItem,
};

AiTypeObject AiType_Dict = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "dict",                                     /* tp_name */
    sizeof(AiDictObject),                       /* tp_basesize */
    0,                                          /* tp_itemsize */
    (destructor)dict_dealloc,                   /* tp_dealloc */
    (printfunc)dict_print,                      /* tp_print */
    0,                                          /* tp_compare */

    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    &dict_as_mapping,                           /* tp_as_mapping */

    (hashfunc)AiObject_Unhashable,              /* tp_hash */
    0,                                          /* tp_call */
    (unaryfunc)dict_str,                        /* tp_str */

    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,//AiObject_Generic_Getattr,                     /* tp_getattro */
    0,                                          /* tp_setattro */

    SUBCLASS_DICT | BASE_TYPE,                  /* tp_flags */

    0,//(unaryfunc)dict_iter,                       /* tp_iter */
    0,                                          /* tp_iternext */

    dict_methods,                               /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,//dict_init,                                  /* tp_init */
    0,//AiType_Generic_Alloc,                         /* tp_alloc */
    0,//dict_New,                                   /* tp_new */
    AiObject_GC_Del,                            /* tp_free */
};

AiObject *dummy;

AiDictEntry *AiDict_Lookup(AiDictObject *mp, AiObject *key, long hash) {
    size_t perturb;
    size_t mask = (size_t)mp->ma_mask;
    size_t i = hash & mask;
    AiDictEntry *ep = &mp->ma_table[i];

    if (!ep->me_key || ep->me_key == key) {
        return ep;
    }
    else if (ep->me_key == dummy) {
        freeslot = ep;
    }
    else {
        if (ep->me_hash == hash) {
            if (AiObject_Rich_Compare(ep->me_key, key, CMP_EQ) > 0) {
                return ep;
            }
        }
        freeslot = NULL;
    }
    for (perturb = hash; ; perturb >>= PERTURB_SHIFT) {
        i = (i << 2) + i + perturb + 1;
        ep = &mp->ma_table[i & mask];
        if (!ep->me_key) {
            return freeslot ? freeslot : ep;
        }
        else if (ep->me_key == key) {
            return ep;
        }
        else if (ep->me_hash == hash && ep->me_key != dummy) {
            if (AiObject_Rich_Compare(ep->me_key, key, CMP_EQ) > 0) {
                return ep;
            }
        }
        else if (ep->me_key == dummy && !freeslot) {
            freeslot = ep;
        }
    }
}

AiDictEntry *AiDict_Lookup_String(AiDictObject *mp, AiStringObject *key, long hash) {
    size_t perturb;
    size_t mask = (size_t)mp->ma_mask;
    size_t i = (size_t)(hash & mask);
    AiDictEntry *ep = &mp->ma_table[i];

    if (!CHECK_TYPE_STRING(key)) {
        mp->ma_lookup = AiDict_Lookup;
        return AiDict_Lookup(mp, (AiObject *)key, hash);
    }

    if (!ep->me_key || ep->me_key == (AiObject *)key) {
        return ep;
    }
    else if (ep->me_key == dummy) {
        freeslot = ep;
    }
    else {
        if (ep->me_hash == hash) {
            if (STRING_EQUAL(ep->me_key, key)) {
                return ep;
            }
        }
        freeslot = NULL;
    }
    for (perturb = (size_t)hash; ; perturb >>= PERTURB_SHIFT) {
        i = (i << 2) + i + perturb + 1;
        ep = &mp->ma_table[i & mask];
        if (!ep->me_key) {
            return freeslot ? freeslot : ep;
        }
        else if (ep->me_key == (AiObject *)key) {
            return ep;
        }
        else if (ep->me_hash == hash && ep->me_key != dummy) {
            if (STRING_EQUAL(ep->me_key, key)) {
                return ep;
            }
        }
        else if (ep->me_key == dummy && !freeslot) {
            freeslot = ep;
        }
    }
}

AiObject *AiDict_New() {
    AiDictObject *mp;
    if (!dummy) {
        dummy = AiString_From_String("<Akari~~~>");
    }
    if (number_free_dicts) {
        mp = free_dicts[--number_free_dicts];
        INIT_REFCNT(mp);
        if(mp->ma_fill) {
            EMPTY_TO_MINSIZE(mp);
        }
    }
    else {
        mp = AiObject_NEW(AiDictObject, &AiType_Dict);
        EMPTY_TO_MINSIZE(mp);
    }
    mp->ma_lookup = (lookupfunc)AiDict_Lookup_String;
    return (AiObject *)mp;
}

AiObject *AiDict_GetItem(AiDictObject *mp, AiObject *key) {
    long hash;

    if (CHECK_TYPE_STRING(key)) {
        hash = ((AiStringObject *)key)->ob_shash;
        if (hash == -1) {
            hash = string_hash((AiStringObject *)key);
        }
    }
    else {
        hash = OBJECT_HASH(key);
        if (hash == -1) {
            RUNTIME_EXCEPTION("bad hash");
            return NULL;
        }
    }

    return mp->ma_lookup(mp, key, hash)->me_value;
}

int AiDict_SetItem(AiDictObject *mp, AiObject *key, AiObject *value) {
    long hash;
    ssize_t used;

    if (CHECK_TYPE_STRING(key)) {
        hash = ((AiStringObject *)key)->ob_shash;
        if (hash == -1) {
            hash = string_hash((AiStringObject *)key);
        }
    }
    else {
        hash = OBJECT_HASH(key);
        if (hash == -1) {
            RUNTIME_EXCEPTION("bad hash");
            return -1;
        }
    }
    used = DICT_SIZE(mp);
    AiDict_Insert(mp, key, hash, value);

    if (DICT_SIZE(mp) > used && mp->ma_fill * 3 >= (mp->ma_mask + 1) * 2) {
        return dict_resize(mp, DICT_SIZE(mp) * (DICT_SIZE(mp) > 50000 ? 2 : 4));
    }
    else {
        return 0;
    }
}

int AiDict_Insert(AiDictObject *mp, AiObject *key, long hash, AiObject *value) {
    AiObject *old_value;
    AiDictEntry *ep;

    ep = mp->ma_lookup(mp, key, hash);
    if (ep->me_value) {
        old_value = ep->me_value;
        ep->me_value = value;
        DEC_REFCNT(old_value);
        INC_REFCNT(value);
    }
    else {
        if (!ep->me_key) {
            ++mp->ma_fill;
        }
        else {
            DEC_REFCNT(ep->me_key);
        }
        INC_REFCNT(key);
        INC_REFCNT(value);
        ep->me_key = key;
        ep->me_hash = hash;
        ep->me_value = value;
        ++DICT_SIZE(mp);
    }

    return 0;
}

int AiDict_DelItem(AiDictObject *mp, AiObject *key) {
    long hash;
    AiDictEntry *ep;
    AiObject *old_key, *old_value;

    if (!CHECK_TYPE_STRING(key)
        || (hash = ((AiStringObject *)key)->ob_shash) == -1) {
        hash = OBJECT_HASH(key);
        if (hash == -1) {
            RUNTIME_EXCEPTION("bad hash");
            return -1;
        }
    }
    ep = mp->ma_lookup(mp, key, hash);
    if (!ep->me_value) {
        return -1;
    }

    old_key = ep->me_key;
    ep->me_key = dummy;
    old_value = ep->me_value;
    ep->me_value = NULL;
    --DICT_SIZE(mp);
    DEC_REFCNT(old_value);
    DEC_REFCNT(old_key);

    return 0;
}

AiObject *dict_str(AiDictObject *mp) {
    AiDictEntry *ep;
    ssize_t used = DICT_SIZE(mp);
    char *p;
    ssize_t i = 0;
    AiListObject *strlist;
    AiStringObject *str;
    AiStringObject *key;
    AiStringObject *value;
    AiStringObject *split;

    if (DICT_SIZE(mp) == 0) {
        return AiString_From_String("{}");
    }
    else if (DICT_SIZE(mp) == 1) {
        for (ep = mp->ma_table; ; ++ep) {
            if (ep->me_key && ep->me_value) {
                key = (AiStringObject *)OB_TO_STRING(ep->me_key);
                value = (AiStringObject *)OB_TO_STRING(ep->me_value);
                break;
            }
        }
        str = (AiStringObject *)AiString_From_StringAndSize(NULL, STRING_LEN(key) + STRING_LEN(value) + 2 + 2);
        STRING_AS_CSTRING(str)[0] = '{';
        p = &STRING_AS_CSTRING(str)[1];
        AiMem_Copy(p, STRING_AS_CSTRING(key), STRING_LEN(key));
        p += STRING_LEN(key);
        *p++ = ':';
        *p++ = ' ';
        AiMem_Copy(p, STRING_AS_CSTRING(value), STRING_LEN(value));
        STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = '}';

        DEC_REFCNT(key);
        DEC_REFCNT(value);

        return (AiObject *)str;
    }

    strlist = (AiListObject *)AiList_New(DICT_SIZE(mp));

    for (ep = mp->ma_table; ; ++ep) {
        if (ep->me_key && ep->me_value) {
            --used;
            key = (AiStringObject *)OB_TO_STRING(ep->me_key);
            value = (AiStringObject *)OB_TO_STRING(ep->me_value);
            ++ep;
            break;
        }
    }
    strlist->ob_item[0] = AiString_From_StringAndSize(NULL, STRING_LEN(key) + STRING_LEN(value) + 2 + 1);
    str = (AiStringObject *)(strlist->ob_item[0]);
    STRING_AS_CSTRING(str)[0] = '{';
    p = &STRING_AS_CSTRING(str)[1];
    AiMem_Copy(p, STRING_AS_CSTRING(key), STRING_LEN(key));
    p += STRING_LEN(key);
    *p++ = ':';
    *p++ = ' ';
    AiMem_Copy(p, STRING_AS_CSTRING(value), STRING_LEN(value));

    DEC_REFCNT(key);
    DEC_REFCNT(value);

    for (; used > 1; ++ep) {
        if (ep->me_key && ep->me_value) {
            --used;
            ++i;
            key = (AiStringObject *)OB_TO_STRING(ep->me_key);
            value = (AiStringObject *)OB_TO_STRING(ep->me_value);

            strlist->ob_item[i] = AiString_From_StringAndSize(NULL, STRING_LEN(key) + STRING_LEN(value) + 2);
            str = (AiStringObject *)(strlist->ob_item[i]);
            p = STRING_AS_CSTRING(str);
            AiMem_Copy(p, STRING_AS_CSTRING(key), STRING_LEN(key));
            p += STRING_LEN(key);
            *p++ = ':';
            *p++ = ' ';
            AiMem_Copy(p, STRING_AS_CSTRING(value), STRING_LEN(value));

            DEC_REFCNT(key);
            DEC_REFCNT(value);
        }
    }

    for (; ; ++ep) {
        if (ep->me_key && ep->me_value) {
            --used;
            key = (AiStringObject *)OB_TO_STRING(ep->me_key);
            value = (AiStringObject *)OB_TO_STRING(ep->me_value);
            break;
        }
    }
    strlist->ob_item[LIST_SIZE(strlist) - 1] = AiString_From_StringAndSize(NULL, STRING_LEN(key) + STRING_LEN(value) + 2 + 1);
    str = (AiStringObject *)(strlist->ob_item[LIST_SIZE(strlist) - 1]);
    p = STRING_AS_CSTRING(str);
    AiMem_Copy(p, STRING_AS_CSTRING(key), STRING_LEN(key));
    p += STRING_LEN(key);
    *p++ = ':';
    *p++ = ' ';
    AiMem_Copy(p, STRING_AS_CSTRING(value), STRING_LEN(value));
    STRING_AS_CSTRING(str)[STRING_LEN(str) - 1] = '}';

    DEC_REFCNT(key);
    DEC_REFCNT(value);

    split = (AiStringObject *)AiString_From_String(", ");
    str = (AiStringObject *)string_join(split, (AiObject *)strlist);

    DEC_REFCNT(split);
    DEC_REFCNT(strlist);

    return (AiObject *)str;
}

int AiDict_ClearAllMemory() {
    while (number_free_dicts--) {
        AiMem_Free(free_dicts[number_free_dicts]);
    }
    if (dummy) {
        AiMem_Free(dummy);
    }
    return 0;
}

ssize_t dict_size(AiDictObject *mp) {
    return DICT_SIZE(mp);
}

int dict_resize(AiDictObject *mp, ssize_t minused) {
    AiDictEntry *old_table;
    AiDictEntry *new_table;
    AiDictEntry *ep;
    AiDictEntry small_copy[DICT_SMALLTABLE_SIZE];
    ssize_t newsize;
    ssize_t i;
    int old_table_is_smalltable;

    for (newsize = DICT_SMALLTABLE_SIZE;
        newsize <= minused && newsize > 0; newsize <<= 1);
    old_table = mp->ma_table;
    old_table_is_smalltable = old_table == mp->ma_smalltable;

    if (newsize == DICT_SMALLTABLE_SIZE) {
        new_table = mp->ma_smalltable;
        if (new_table == old_table) {
            if (mp->ma_fill == DICT_SIZE(mp)) {
                return 0;
            }
            else {
                AiMem_Copy(small_copy, old_table, sizeof(small_copy));
                old_table = small_copy;
            }
        }
    }
    else {
        new_table = AiMem_Alloc(sizeof(AiDictEntry) * newsize);
    }

    mp->ma_table = new_table;
    mp->ma_mask = newsize - 1;
    AiMem_Set(new_table, 0, sizeof(AiDictEntry) * newsize);
    DICT_SIZE(mp) = 0;
    i = mp->ma_fill;
    mp->ma_fill = 0;

    for (ep = old_table; i > 0; ++ep) {
        if (ep->me_value) {
            --i;
            AiDict_Insert(mp, ep->me_key, ep->me_hash, ep->me_value);
        }
        else if (ep->me_key) {
            --i;
            assert(ep->me_key == dummy);
            DEC_REFCNT(ep->me_key);
        }
    }
    if (old_table_is_smalltable) {
        AiMem_Free(old_table);
    }
    return 0;
}

void dict_dealloc(AiDictObject *mp) {
    AiDictEntry *ep;
    ssize_t fill = mp->ma_fill;

    for (ep = mp->ma_table; fill > 0; ++ep) {
        if (ep->me_key) {
            --fill;
            DEC_REFCNT(ep->me_key);
            XDEC_REFCNT(ep->me_value);
        }
    }
    if (mp->ma_table != mp->ma_smalltable) {
        AiMem_Free(mp->ma_table);
    }
    if (CHECK_EXACT_TYPE_DICT(mp) && number_free_dicts < NUMBER_FREE_DICTS_MAX) {
        free_dicts[number_free_dicts++] = mp;
    }
    else {
        OB_FREE(mp);
    }
}

void dict_print(AiDictObject *mp, FILE *stream) {
    AiDictEntry *ep;
    ssize_t used = DICT_SIZE(mp);

    fputs("{", stream);
    for (ep = mp->ma_table; used > 1; ++ep) {
        if (ep->me_key && ep->me_value) {
            --used;
            OB_PRINT(ep->me_key, stream);
            fputs(": ", stream);
            OB_PRINT(ep->me_value, stream);
            fputs(", ", stream);
        }
    }
    for (; ; ++ep) {
        if (ep->me_key && ep->me_value) {
            OB_PRINT(ep->me_key, stream);
            fputs(": ", stream);
            OB_PRINT(ep->me_value, stream);
            break;
        }
    }
    fputs("}", stream);
}
