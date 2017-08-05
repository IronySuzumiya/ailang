#pragma once
#ifndef AI_MEMORY_H
#define AI_MEMORY_H

#include "../aiconfig.h"

#define AiMem_Alloc             mem_alloc
#define AiMem_Free              mem_free
#define AiMem_Realloc           mem_realloc
#define AiMem_Copy              mem_copy
#define AiMem_Set               mem_set
#define AiObject_GC_Del         AiMem_Free
#define AiObject_GC_Free        AiMem_Free

#define AiObject_INIT(ob, type)     \
    (OB_TYPE(ob) = (type), INIT_REFCNT(ob), (ob))
#define AiVarObject_INIT(ob, type, size)    \
    (OB_SIZE(ob) = (size), AiObject_INIT(ob, type))

#define _AiObject_SIZE(type) ((type)->tp_basicsize)
#define _AiVarObject_SIZE(type, nitems)     \
    ((size_t)((type)->tp_basicsize + (nitems) * (type)->tp_itemsize))
#define AiObject_SIZE(ob) _AiObject_SIZE((ob)->ob_type)
#define AiVarObject_SIZE(ob, nitems) _AiVarObject_SIZE((ob)->ob_type, (nitems))

#define AiObject_NEW(type, typeobj)         \
    ((type *)AiObject_Init(                 \
        (AiObject *)AiMem_Alloc(            \
            _AiObject_SIZE(typeobj)         \
        ),                                  \
        (typeobj)                           \
    ))
#define AiVarObject_NEW(type, typeobj, n)       \
    ((type *)AiVarObject_Init(                  \
        (AiVarObject *)AiMem_Alloc(             \
            _AiVarObject_SIZE((typeobj), (n))   \
        ),                                      \
        (typeobj),                              \
        (n)                                     \
    ))

typedef struct _heap_list {
    void *mem;
    struct _heap_list *next;
}
HeapList;

AiAPI_DATA(HeapList *) heaphead;
AiAPI_FUNC(void *) mem_alloc(ssize_t size);
AiAPI_FUNC(void) mem_free(void *p);
AiAPI_FUNC(void *) mem_realloc(void *p, ssize_t size);
AiAPI_FUNC(void) mem_copy(void *dst, void *src, ssize_t size);
AiAPI_FUNC(void) mem_set(void *p, int val, ssize_t size);
AiAPI_FUNC(AiObject *) AiObject_Init(AiObject *ob, AiTypeObject *type);
AiAPI_FUNC(AiVarObject *) AiVarObject_Init(AiVarObject *ob, AiTypeObject *type, ssize_t size);

#endif
