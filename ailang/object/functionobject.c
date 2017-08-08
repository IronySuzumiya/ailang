#include "../ailang.h"

static void function_dealloc(AiFunctionObject *func);
static void cfunction_dealloc(AiCFunctionObject *func);

AiTypeObject AiType_Function = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "function",                         /* tp_name */
    sizeof(AiFunctionObject),           /* tp_basicsize */
    0,                                  /* tp_itemsize */

    (destructor)function_dealloc,       /* tp_dealloc */
    0,                                  /* tp_compare */
    0,                                  /* tp_hash */
    0,//function_call,                      /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//function_new,                       /* tp_new */
};

AiObject *AiFunction_New(AiObject *code, AiObject *globals) {
    static AiObject *__name__ = NULL;
    AiObject *consts;
    AiObject *module;
    AiFunctionObject *func = AiObject_NEW(AiFunctionObject, &AiType_Function);

    func->func_code = code;
    INC_REFCNT(code);
    func->func_globals = globals;
    INC_REFCNT(globals);
    func->func_name = ((AiCodeObject *)code)->co_name;
    INC_REFCNT(func->func_name);
    func->func_defaults = NULL;
    func->func_closure = NULL;
    consts = ((AiCodeObject *)code)->co_consts;
    func->func_dict = NULL;
    func->func_module = NULL;
    if (!__name__) {
        __name__ = AiString_From_String("__name__");
        AiString_Intern((AiStringObject **)&__name__);
    }
    module = AiDict_GetItem((AiDictObject *)globals, __name__);
    if (module) {
        INC_REFCNT(module);
        func->func_module = module;
    }
    return (AiObject *)func;
}

int AiFunction_SetDefaults(AiFunctionObject *func, AiObject *defaults) {
    if (defaults == NONE) {
        defaults = NULL;
    }
    else if (defaults && CHECK_EXACT_TYPE_TUPLE(defaults)) {
        INC_REFCNT(defaults);
    }
    else {
        RUNTIME_EXCEPTION("non-tuple default args");
        return -1;
    }
    XDEC_REFCNT(func->func_defaults);
    func->func_defaults = defaults;
    return 0;
}

int AiFunction_SetClosure(AiFunctionObject *func, AiObject *closure) {
    if (closure == NONE) {
        closure = NULL;
    }
    else if (closure && CHECK_EXACT_TYPE_TUPLE(closure)) {
        INC_REFCNT(closure);
    }
    else {
        RUNTIME_EXCEPTION("expected tuple for closure, got '%s'", OB_TYPENAME(closure));
        return -1;
    }
    XDEC_REFCNT(func->func_closure);
    func->func_closure = closure;
    return 0;
}

AiTypeObject AiType_CFunction = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "function",                         /* tp_name */
    sizeof(AiCFunctionObject),          /* tp_basicsize */
    0,                                  /* tp_itemsize */

    (destructor)cfunction_dealloc,      /* tp_dealloc */
    0,                                  /* tp_compare */
    0,                                  /* tp_hash */
    0,//cfunction_call,                     /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//cfunction_new,                      /* tp_new */
};

AiObject *AiCFunction_New(AiCFunction func) {
    AiCFunctionObject *f = (AiCFunctionObject *)AiObject_NEW(AiCFunctionObject, &AiType_CFunction);
    f->func = func;
    return (AiObject *)f;
}

void function_dealloc(AiFunctionObject *func) {
    // ignore weakrefs yet
    DEC_REFCNT(func->func_code);
    DEC_REFCNT(func->func_globals);
    XDEC_REFCNT(func->func_module);
    DEC_REFCNT(func->func_name);
    XDEC_REFCNT(func->func_defaults);
    XDEC_REFCNT(func->func_dict);
    XDEC_REFCNT(func->func_closure);
    AiObject_Del(func);
}

void cfunction_dealloc(AiCFunctionObject *func) {
    AiObject_Del(func);
}
