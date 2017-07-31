#pragma once
#ifndef DICT_OBJECT_H
#define DICT_OBJECT_H

#include "../system/utils.h"

#define DICT_SMALL_TABLE_SIZE 8

typedef struct _dictentry {
    long me_hash;
    AiObject *me_key;
    AiObject *me_value;
}
AiDictEntry;

typedef AiDictEntry *(*lookupfunc)(struct _dictobject *mp, AiObject *key, long hash);

typedef struct _dictobject {
    OBJECT_HEAD
    ssize_t ma_fill;
    ssize_t ma_used;
    ssize_t ma_mask;
    AiDictEntry *ma_table;
    lookupfunc ma_lookup;
    AiDictEntry ma_smalltable[DICT_SMALL_TABLE_SIZE];
}
AiDictObject;

#define INIT_NONZERO_DICT_SLOTS(mp)                 \
    WRAP(                                           \
        (mp)->ma_table = (mp)->ma_smalltable;       \
        (mp)->ma_mask = DICT_SMALL_TABLE_SIZE - 1;  \
    )

#define EMPTY_TO_MINSIZE(mp)                                            \
    WRAP(                                                               \
        AiMEM_SET((mp)->ma_smalltable, 0, sizeof((mp)->ma_smalltable)); \
        DICT_SIZE(mp) = (mp)->ma_fill = 0;                              \
        INIT_NONZERO_DICT_SLOTS(mp);                                    \
    )

#define NUMBER_FREE_DICTS_MAX 80

#define PERTURB_SHIFT 5

#define CHECK_TYPE_DICT(ob) CHECK_TYPE(ob, &type_dictobject)

#define DICT_SIZE(ob) (((AiDictObject *)(ob))->ma_used)

AiAPI_DATA(AiTypeObject) type_dictobject;
AiAPI_DATA(AiObject *) dummy;
AiAPI_DATA(AiDictEntry *) freeslot;
AiAPI_DATA(AiDictObject *) free_dicts[NUMBER_FREE_DICTS_MAX];
AiAPI_DATA(int) number_free_dicts;
AiAPI_FUNC(AiDictEntry *) dict_lookup(AiDictObject *mp, AiObject *key, long hash);
AiAPI_FUNC(AiDictEntry *) dict_lookup_with_string(AiDictObject *mp, AiStringObject *key, long hash);
AiAPI_FUNC(AiObject *) dict_new(void);
AiAPI_FUNC(AiObject *) dict_getitem(AiDictObject *mp, AiObject *key);
AiAPI_FUNC(int) dict_setitem(AiDictObject *mp, AiObject *key, AiObject *value);
AiAPI_FUNC(int) dict_insert(AiDictObject *mp, AiObject *key, long hash, AiObject *value);
AiAPI_FUNC(int) dict_delitem(AiDictObject *mp, AiObject *key);
AiAPI_FUNC(AiObject *) dict_to_string(AiDictObject *mp);

AiAPI_FUNC(int) dict_clear_free_dicts(void);

#endif
