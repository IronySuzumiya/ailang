#pragma once
#ifndef AI_CONFIG_H
#define AI_CONFIG_H

#ifdef _DEBUG
#define AI_DEBUG
#endif

#define AI_MAGIC (0x23 | (0x33 << 8))

typedef __int64 ssize_t;

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

#endif
