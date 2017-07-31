#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>

#define AiAPI_FUNC(RTYPE) RTYPE
#define AiAPI_DATA(RTYPE) extern RTYPE

#define WRAP(x) do { x } while(0)

AiAPI_FUNC(void) inline print_error_or_exception(char *type, char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, type);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

AiAPI_FUNC(void) inline print_runtime_exception(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "runtime exception: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

AiAPI_FUNC(void) inline print_type_error(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "type error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

AiAPI_FUNC(void) inline print_fatal_error(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "fatal error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    abort();
}

#define ERROR_OR_EXCEPTION print_error_or_exception

#define RUNTIME_EXCEPTION print_runtime_exception

#define TYPE_ERROR print_type_error

#define FATAL_ERROR print_fatal_error

#define UNSUPPORTED_BINARY(lhs, rhs, op)    \
    TYPE_ERROR                              \
    ("unsupported binary operation '%s' "   \
        "between '%s' and '%s'",            \
        op, lhs, rhs)

#define UNSUPPORTED_UNARY(a, op)            \
    TYPE_ERROR                              \
    ("unsupported unary operation '%s' "    \
        "on '%s'", op, a)

#define UNSUPPORTED_1ARG_BUILTINFUNC(a, func)   \
    TYPE_ERROR                                  \
    ("unsupported built-in function '%s' "      \
        "on '%s'", func, a)

#define UNSUPPORTED_2ARG_BUILTINFUNC(a, b, func)    \
    TYPE_ERROR                                      \
    ("unsupported built-in function '%s' "          \
        "on '%s' and '%s'", func, a, b)

#define UNSUPPORTED_ADD(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "+")

#define UNSUPPORTED_SUB(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "-")

#define UNSUPPORTED_MUL(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "*")

#define UNSUPPORTED_DIV(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "/")

#define UNSUPPORTED_MOD(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "%")

#define UNSUPPORTED_POW(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "**")

#define UNSUPPORTED_POS(a)                  \
    UNSUPPORTED_UNARY(a, "+")

#define UNSUPPORTED_NEG(a)                  \
    UNSUPPORTED_UNARY(a, "-")

#define UNSUPPORTED_ABS(a)                  \
    UNSUPPORTED_1ARG_BUILTINFUNC(a, "abs")

#define UNSUPPORTED_SHL(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "<<")

#define UNSUPPORTED_SHR(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, ">>")

#define UNSUPPORTED_AND(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "&")

#define UNSUPPORTED_OR(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, "|")

#define UNSUPPORTED_NOT(a)                  \
    UNSUPPORTED_UNARY(a, "~")

#define UNSUPPORTED_XOR(lhs, rhs)           \
    UNSUPPORTED_BINARY(lhs, rhs, "^")

#define UNSUPPORTED_CONCAT(fo, la)          \
    UNSUPPORTED_2ARG_BUILTINFUNC(fo, la, "concat")

#define UNSUPPORTED_EXTEND(fo, la)          \
    UNSUPPORTED_2ARG_BUILTINFUNC(fo, la, "extend")

#define UNSUPPORTED_CONTAINS(ob, el)        \
    UNSUPPORTED_2ARG_BUILTINFUNC(ob, el, "contains")

#define UNSUPPORTED_EQ(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, "==")

#define UNSUPPORTED_NE(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, "!=")

#define UNSUPPORTED_GT(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, ">")

#define UNSUPPORTED_LT(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, "<")

#define UNSUPPORTED_GE(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, ">=")

#define UNSUPPORTED_LE(lhs, rhs)            \
    UNSUPPORTED_BINARY(lhs, rhs, "<=")

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

typedef __int64 ssize_t;

#endif
