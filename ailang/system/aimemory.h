#pragma once
#ifndef AI_MEMORY_H
#define AI_MEMORY_H

#include "../system/utils.h"

#define AiMEM_ALLOC     mem_alloc
#define AiMEM_FREE      mem_free
#define AiMEM_REALLOC   mem_realloc
#define AiMEM_COPY      mem_copy
#define AiMEM_SET       mem_set

AiAPI_FUNC(void *) mem_alloc(ssize_t size);
AiAPI_FUNC(void) mem_free(void *p);
AiAPI_FUNC(void *) mem_realloc(void *p, ssize_t size);
AiAPI_FUNC(void) mem_copy(void *dst, void *src, ssize_t size);
AiAPI_FUNC(void) mem_set(void *p, int val, ssize_t size);

#endif
