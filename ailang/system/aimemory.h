#pragma once
#ifndef AI_MEMORY_H
#define AI_MEMORY_H

#include "../aiconfig.h"

#define AiMem_Alloc             mem_alloc
#define AiMem_Free              mem_free
#define AiMem_Realloc           mem_realloc
#define AiMem_Copy              mem_copy
#define AiMem_Set               mem_set
#define AiObject_GC_New(type)   (mem_alloc(sizeof(type)))
#define AiObject_GC_Del         AiMem_Free
#define AiObject_GC_Free        AiMem_Free

typedef struct _heap_list {
    void *mem;
    struct _heap_list *next;
}
HeapList;

AiAPI_FUNC(void *) mem_alloc(ssize_t size);
AiAPI_FUNC(void) mem_free(void *p);
AiAPI_FUNC(void *) mem_realloc(void *p, ssize_t size);
AiAPI_FUNC(void) mem_copy(void *dst, void *src, ssize_t size);
AiAPI_FUNC(void) mem_set(void *p, int val, ssize_t size);
AiAPI_DATA(HeapList *) heaphead;

#endif
