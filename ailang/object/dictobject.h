#pragma once
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include "../aiconfig.h"

#define DICT_SMALLTABLE_SIZE 8

typedef struct _dictentry {
    long me_hash;
    AiObject *me_key;
    AiObject *me_value;
}
AiDictEntry;

typedef AiDictEntry *(*lookupfunc)(struct _dictobject *mp, AiObject *key, long hash);

typedef struct _dictobject {
    AiObject_HEAD
    ssize_t ma_fill;
    ssize_t ma_used;
    ssize_t ma_mask;
    AiDictEntry *ma_table;
    lookupfunc ma_lookup;
    AiDictEntry ma_smalltable[DICT_SMALLTABLE_SIZE];
}
AiDictObject;

#define INIT_NONZERO_DICT_SLOTS(mp)                 \
    WRAP(                                           \
        (mp)->ma_table = (mp)->ma_smalltable;       \
        (mp)->ma_mask = DICT_SMALLTABLE_SIZE - 1;   \
    )

#define EMPTY_TO_MINSIZE(mp)                                            \
    WRAP(                                                               \
        AiMem_Set((mp)->ma_smalltable, 0, sizeof((mp)->ma_smalltable)); \
        DICT_SIZE(mp) = (mp)->ma_fill = 0;                              \
        INIT_NONZERO_DICT_SLOTS(mp);                                    \
    )

#define NUMBER_FREE_DICTS_MAX 80

#define PERTURB_SHIFT 5

#define CHECK_EXACT_TYPE_DICT(ob) CHECK_TYPE(ob, &AiType_Dict)
#define CHECK_TYPE_DICT(ob) CHECK_FAST_SUBCLASS(ob, SUBCLASS_DICT)

#define DICT_SIZE(ob) (((AiDictObject *)(ob))->ma_used)

#define DICT_GETITEM(dict, key) AiDict_GetItem((AiDictObject *)(dict), (key))
#define DICT_GETITEM_WITHSTRING(dict, str) AiDict_GetItem_WithString((AiDictObject *)(dict), (str))

AiAPI_DATA(AiTypeObject) AiType_Dict;
AiAPI_FUNC(AiDictEntry *) AiDict_Lookup(AiDictObject *mp, AiObject *key, long hash);
AiAPI_FUNC(AiDictEntry *) AiDict_Lookup_String(AiDictObject *mp, AiStringObject *key, long hash);
AiAPI_FUNC(AiObject *) AiDict_New(void);
AiAPI_FUNC(AiObject *) AiDict_GetItem(AiDictObject *mp, AiObject *key);
AiAPI_FUNC(int) AiDict_SetItem(AiDictObject *mp, AiObject *key, AiObject *value);
AiAPI_FUNC(int) AiDict_Insert(AiDictObject *mp, AiObject *key, long hash, AiObject *value);
AiAPI_FUNC(int) AiDict_DelItem(AiDictObject *mp, AiObject *key);
AiAPI_FUNC(int) AiDict_Merge(AiDictObject *dist, AiDictObject *src, int override);
AiAPI_FUNC(AiObject *) AiDict_Copy(AiDictObject *o);
AiAPI_FUNC(AiObject *) AiDict_GetItem_WithString(AiDictObject *dict, char *str);
AiAPI_FUNC(int) AiDict_SetItem_WithString(AiDictObject *dict, char *key, AiObject *value);

AiAPI_FUNC(int) AiDict_ClearAllMemory(void);

#endif
