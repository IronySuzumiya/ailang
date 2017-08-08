#include "../ailang.h"

static AiObject _runtime_exception = {
    AiObject_HEAD_INIT(&AiType_Exception)
};

static AiObject _type_error = {
    AiObject_HEAD_INIT(&AiType_Exception)
};

AiTypeObject AiType_BaseException = {
    AiVarObject_HEAD_INIT(NULL, 0)
    EXC_MODULE_NAME "BaseException",    /* tp_name */
    
};

AiTypeObject AiType_Exception = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "exception",
};

AiObject *runtime_exception = &_runtime_exception;
AiObject *type_error = &_type_error;

void AiException_Restore(AiObject *type, AiObject *value, AiObject *traceback) {
    AiThreadState *tstate = AiThreadState_Get();

    XDEC_REFCNT(tstate->curexc_type);
    XDEC_REFCNT(tstate->curexc_value);
    XDEC_REFCNT(tstate->curexc_traceback);

    tstate->curexc_type = type;
    tstate->curexc_value = value;
    tstate->curexc_traceback = traceback;
}

void AiException_SetObject(AiObject *exception, AiObject *value) {
    XINC_REFCNT(exception);
    XINC_REFCNT(value);
    AiException_Restore(exception, value, NULL);
}

void AiException_SetString(AiObject *exception, char *string) {
    AiObject *value = AiString_From_String(string);
    AiException_SetObject(exception, value);
    XDEC_REFCNT(value);
}

void AiException_Clear() {
    AiException_Restore(NULL, NULL, NULL);
}

void AiException_Fetch(AiObject **type, AiObject **value, AiObject **tb) {
    AiThreadState *tstate = AiThreadState_Get();

    *type = tstate->curexc_type;
    *value = tstate->curexc_value;
    *tb = tstate->curexc_traceback;

    tstate->curexc_type = NULL;
    tstate->curexc_value = NULL;
    tstate->curexc_traceback = NULL;
}

int AiExceptionClass_Check(AiObject *exception) {
    return exception == runtime_exception || exception == type_error;
}

int AiException_Matches(AiObject *err, AiObject *exc) {
    if (CHECK_EXACT_TYPE_TUPLE(exc)) {
        for (ssize_t i = 0; i < TUPLE_SIZE(exc); ++i) {
            if (AiException_Matches(err, TUPLE_GETITEM(exc, i))) {
                return 1;
            }
        }
        return 0;
    }
    else if (AiExceptionClass_Check(err) && AiExceptionClass_Check(exc)) {
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
    AiException_SetString(runtime_exception, buffer);

    va_end(ap);
}

void type_error_restore(char *msg, ...) {
    char buffer[100];
    va_list ap;
    va_start(ap, msg);

    vsprintf_s(buffer, 100, msg, ap);
    AiException_SetString(type_error, buffer);

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
