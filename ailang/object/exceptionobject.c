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

void exception_fetch(AiObject **type, AiObject **value, AiObject **tb) {
    AiThreadState *tstate = threadstate_get();

    *type = tstate->curexc_type;
    *value = tstate->curexc_value;
    *tb = tstate->curexc_traceback;

    tstate->curexc_type = NULL;
    tstate->curexc_value = NULL;
    tstate->curexc_traceback = NULL;
}

int exceptionclass_check(AiObject *exception) {
    return exception == runtime_exception || exception == type_error;
}

int exception_matches(AiObject *err, AiObject *exc) {
    if (CHECK_TYPE_TUPLE(exc)) {
        for (ssize_t i = 0; i < TUPLE_SIZE(exc); ++i) {
            if (exception_matches(err, TUPLE_GET_ITEM(exc, i))) {
                return 1;
            }
        }
        return 0;
    }
    else if (exceptionclass_check(err) && exceptionclass_check(exc)) {
        return err == exc;
    }
    else {
        return 0;
    }
}

void runtime_exception_store(char *msg, ...) {
    char buffer[100];
    va_list ap;
    va_start(ap, msg);

    vsprintf_s(buffer, 100, msg, ap);
    exception_setstring(runtime_exception, buffer);

    va_end(ap);
}

void type_error_restore(char *msg, ...) {
    char buffer[100];
    va_list ap;
    va_start(ap, msg);

    vsprintf_s(buffer, 100, msg, ap);
    exception_setstring(type_error, buffer);

    va_end(ap);
}

void fatal_error_abort(char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    fprintf(stderr, "fatal error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    abort();
}
