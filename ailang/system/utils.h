#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>

#define API_FUNC(RTYPE) RTYPE
#define API_DATA(RTYPE) extern RTYPE

#define WRAP(x) do { x } while(0)

API_FUNC(void) inline print_error_or_exception(const char *type, const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, type);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

API_FUNC(void) inline print_runtime_exception(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "runtime exception: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

API_FUNC(void) inline print_type_error(const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "type error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

API_FUNC(void) inline print_fatal_error(const char *msg, ...) {
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

#define UNSUPPORTED_BUILTINFUNC(a, func)    \
    TYPE_ERROR                              \
    ("unsupported built-in function '%s' "  \
        "on '%s'", func, a)

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
    UNSUPPORTED_BUILTINFUNC(a, "abs")

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

#define PRINT_STDOUT(ob) ((ob)->ob_type->tp_print(ob, stdout))

typedef __int64 ssize_t;

#endif
