#pragma once
#ifndef EXCEPTION_OBJECT_H
#define EXCEPTION_OBJECT_H

#include "../aiconfig.h"

#define RUNTIME_EXCEPTION runtime_exception_store

#define TYPE_ERROR type_error_restore

#define FATAL_ERROR fatal_error_abort

#define UNSUPPORTED_BINARY(lhs, rhs, op)    \
    TYPE_ERROR                              \
    ("unsupported binary operation '%s' "   \
        "between '%s' and '%s'",            \
        op, OB_TYPENAME(lhs), OB_TYPENAME(rhs))

#define UNSUPPORTED_UNARY(a, op)            \
    TYPE_ERROR                              \
    ("unsupported unary operation '%s' "    \
        "on '%s'", op, OB_TYPENAME(a))

#define UNSUPPORTED_1ARG_BUILTINFUNC(a, func)   \
    TYPE_ERROR                                  \
    ("unsupported built-in function '%s' "      \
        "on '%s'", func, OB_TYPENAME(a))

#define UNSUPPORTED_2ARG_BUILTINFUNC(a, b, func)    \
    TYPE_ERROR                                      \
    ("unsupported built-in function '%s' "          \
        "on '%s' and '%s'", func, OB_TYPENAME(a), OB_TYPENAME(b))

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

#define UNSUPPORTED_NONZERO(a)              \
    UNSUPPORTED_UNARY(a, "?")

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

#define AiException_HEAD  \
    AiObject_HEAD         \
    AiObject *dict;     \
    AiObject *args;     \
    AiObject *message;

#define EXC_MODULE_NAME "exception."

typedef struct _baseexceptionobject {
    AiException_HEAD
}
AiBaseExceptionObject;

typedef struct _syntaxerrorobject {
    AiException_HEAD
    AiObject *msg;
    AiObject *filename;
    AiObject *lineno;
    AiObject *offset;
    AiObject *text;
    AiObject *print_file_and_line;
}
AiSyntaxErrorObject;

typedef struct _systemexitobject {
    AiException_HEAD
    AiObject *code;
}
AiSystemExitObject;

typedef struct _environmenterrorobject {
    AiException_HEAD
    AiObject *myerrno;
    AiObject *strerror;
    AiObject *filename;
}
AiEnvironmentErrorObject;

#define EXCEPTION_OCCURRED() (threadstate_get()->curexc_type)

AiAPI_DATA(struct _typeobject) type_exceptionobject;
AiAPI_DATA(AiObject *) runtime_exception;
AiAPI_DATA(AiObject *) type_error;

AiAPI_FUNC(void) exception_restore(AiObject *type, AiObject *value, AiObject *traceback);
AiAPI_FUNC(void) exception_setobject(AiObject *exception, AiObject *value);
AiAPI_FUNC(void) exception_setstring(AiObject *exception, char *string);
AiAPI_FUNC(void) exception_clear();
AiAPI_FUNC(void) exception_fetch(AiObject **type, AiObject **value, AiObject **tb);
AiAPI_FUNC(int) exceptionclass_check(AiObject *exception);
AiAPI_FUNC(int) exception_matches(AiObject *err, AiObject *exc);

AiAPI_FUNC(void) runtime_exception_store(char *msg, ...);
AiAPI_FUNC(void) type_error_restore(char *msg, ...);
AiAPI_FUNC(void) fatal_error_abort(char *msg, ...);

#endif
