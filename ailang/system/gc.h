#pragma once
#ifndef GC_H
#define GC_H

#include "../system/utils.h"

API_FUNC(void *) gc_malloc(ssize_t size);
API_FUNC(void) gc_free(void *p);

#endif
