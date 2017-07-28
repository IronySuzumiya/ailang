#pragma once
#ifndef UTILS_H
#define UTILS_H

#define API_FUNC(RTYPE) RTYPE
#define API_DATA(RTYPE) extern RTYPE

#define WRAP(x) do { x } while(0)

#define FATAL_ERROR(msg)                    \
    WRAP(                                   \
        fprintf(stderr, "fatal error: ");   \
        fprintf(stderr, msg);               \
        fprintf(stderr, "\n");              \
        abort();                            \
    )

#define PRINT_STDOUT(ob) (ob)->ob_type->tp_print(ob, stdout)

typedef __int64 ssize_t;

#endif
