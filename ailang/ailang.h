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
#include <Windows.h>
#include "system\aiopcode.h"
#include "object\object.h"
#include "object\abstract.h"
#include "system\aimemory.h"
#include "system\aistate.h"
#include "object\typeobject.h"
#include "object\codeobject.h"
#include "object\frameobject.h"
#include "object\exceptionobject.h"
#include "object\tracebackobject.h"
#include "object\intobject.h"
#include "object\boolobject.h"
#include "object\stringobject.h"
#include "object\listobject.h"
#include "object\dictobject.h"
#include "object\iterobject.h"
#include "object\tupleobject.h"
#include "system\aimarshal.h"
#include "system\aieval.h"

#endif
