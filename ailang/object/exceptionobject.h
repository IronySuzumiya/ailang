#pragma once
#ifndef EXCEPTION_OBJECT_H
#define EXCEPTION_OBJECT_H

#include "../system/utils.h"

#define RUNTIME_EXCEPTION record_runtime_exception

#define TYPE_ERROR record_type_error

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

#define EXCEPTION_OCCURRED() (threadstate_get()->curexc_type)

AiAPI_DATA(struct _typeobject) type_exceptionobject;
AiAPI_DATA(AiObject *) runtime_exception;
AiAPI_DATA(AiObject *) type_error;
AiAPI_FUNC(void) exception_restore(AiObject *type, AiObject *value, AiObject *traceback);
AiAPI_FUNC(void) exception_setobject(AiObject *exception, AiObject *value);
AiAPI_FUNC(void) exception_setstring(AiObject *exception, char *string);
AiAPI_FUNC(void) exception_clear();

AiAPI_FUNC(void) record_runtime_exception(char *msg, ...);
AiAPI_FUNC(void) record_type_error(char *msg, ...);
AiAPI_FUNC(void) print_fatal_error(char *msg, ...);

#endif
