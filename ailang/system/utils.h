#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>

#define AiAPI_FUNC(RTYPE) RTYPE
#define AiAPI_DATA(RTYPE) extern RTYPE

#define WRAP(x) do { x } while(0)

#define MAKE_INDEX_IN_RANGE(index, range)   \
    WRAP(                                   \
        if((index) < 0) {                   \
            index += (range);               \
            if((index) < 0) {               \
                index = 0;                  \
            }                               \
        }                                   \
    )

#define OB_PRINT_STDOUT(ob) (OB_PRINT(ob, stdout), fputc('\n', stdout))

#endif
