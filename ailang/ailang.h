#pragma once
#ifndef AI_LANG_H
#define AI_LANG_H

#define AI_DEBUG

#define AI_MAGIC (0x23 | (0x33 << 8))

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "system\utils.h"
#include "object\object.h"
#include "object\typeobject.h"
#include "object\codeobject.h"
#include "system\aimemory.h"
#include "object\intobject.h"
#include "object\stringobject.h"
#include "object\dictobject.h"
#include "object\listobject.h"
#include "object\boolobject.h"
#include "object\tupleobject.h"
#include "system\marshal.h"

#endif
