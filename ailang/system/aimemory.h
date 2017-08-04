#pragma once
#ifndef AI_MEMORY_H
#define AI_MEMORY_H

#include "../aiconfig.h"

#define AiMEM_ALLOC             mem_alloc
#define AiMEM_FREE              mem_free
#define AiMEM_REALLOC           mem_realloc
#define AiMEM_COPY              mem_copy
#define AiMEM_SET               mem_set
#define AiObject_GC_NEW(type)   (mem_alloc(sizeof(type)))
#define AiObject_GC_DEL         AiMEM_FREE
#define AiObject_GC_FREE        AiMEM_FREE

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
