#include "../ailang.h"

static AiObject _runtime_exception = {
    INIT_OBJECT_HEAD(&type_exceptionobject)
};

static AiObject _type_error = {
    INIT_OBJECT_HEAD(&type_exceptionobject)
};

AiTypeObject type_exceptionobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "exception",
};

AiObject *runtime_exception;
AiObject *type_error;

void record_runtime_exception(char *msg, ...) {
    char buffer[100];
    va_list ap;
    va_start(ap, msg);

    vsprintf_s(buffer, 100, msg, ap);
    exception_setstring(runtime_exception, buffer);

    va_end(ap);
}

void record_type_error(char *msg, ...) {
    char buffer[100];
    va_list ap;
    va_start(ap, msg);

    vsprintf_s(buffer, 100, msg, ap);
    exception_setstring(type_error, buffer);

    va_end(ap);
}

void print_fatal_error(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "fatal error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    abort();
}

void exception_restore(AiObject *type, AiObject *value, AiObject *traceback) {
    AiThreadState *tstate = threadstate_get();

    XDEC_REFCNT(tstate->curexc_type);
    XDEC_REFCNT(tstate->curexc_value);
    XDEC_REFCNT(tstate->curexc_traceback);

    tstate->curexc_type = type;
    tstate->curexc_value = value;
    tstate->curexc_traceback = traceback;
}

void exception_setobject(AiObject *exception, AiObject *value) {
    XINC_REFCNT(exception);
    XINC_REFCNT(value);
    exception_restore(exception, value, NULL);
}

void exception_setstring(AiObject *exception, char *string) {
    AiObject *value = string_from_cstring(string);
    exception_setobject(exception, value);
    XDEC_REFCNT(value);
}

void exception_clear() {
    exception_restore(NULL, NULL, NULL);
}
