#include "../ailang.h"

static void function_dealloc(AiFunctionObject *func);

AiTypeObject type_functionobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "function",                         /* tp_name */
    sizeof(AiFunctionObject),           /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)function_dealloc,       /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_hash */
    0,//function_call,                      /* tp_call */
    0,                                  /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//object_generic_getattr,             /* tp_getattro */
    0,//object_generic_setattr,             /* tp_setattro */

    0,                                  /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_methods */
    0,//function_memberlist,                /* tp_members */
    0,//function_getsetlist,                /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,//function_descr_get,                 /* tp_descr_get */
    0,                                  /* tp_descr_set */
    offsetof(
        AiFunctionObject, func_dict),   /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,//function_new,                       /* tp_new */
};

AiObject *function_new(AiObject *code, AiObject *globals) {
    static AiObject *__name__ = NULL;
    AiObject *doc;
    AiObject *consts;
    AiObject *module;
    AiFunctionObject *func = AiObject_GC_NEW(AiFunctionObject);
    INIT_OBJECT(func, &type_functionobject);

    func->func_weakreflist = NULL;
    func->func_code = code;
    INC_REFCNT(code);
    func->func_globals = globals;
    INC_REFCNT(globals);
    func->func_name = ((AiCodeObject *)code)->co_name;
    INC_REFCNT(func->func_name);
    func->func_defaults = NULL;
    func->func_closure = NULL;
    consts = ((AiCodeObject *)code)->co_consts;
    if (tuple_size((AiTupleObject *)consts) >= 1) {
        doc = tuple_getitem((AiTupleObject *)consts, 0);
        if (!CHECK_TYPE_STRING(doc)) {
            doc = NONE;
        }
    }
    else {
        doc = NONE;
    }
    INC_REFCNT(doc);
    func->func_doc = doc;
    func->func_dict = NULL;
    func->func_module = NULL;
    if (!__name__) {
        __name__ = string_from_cstring("__name__");
        string_intern((AiStringObject **)&__name__);
    }
    module = dict_getitem((AiDictObject *)globals, __name__);
    if (module) {
        INC_REFCNT(module);
        func->func_module = module;
    }
    return (AiObject *)func;
}

int function_setdefaults(AiFunctionObject *func, AiObject *defaults) {
    if (defaults == NONE) {
        defaults = NULL;
    }
    else if (defaults && CHECK_TYPE_TUPLE(defaults)) {
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

int function_setclosure(AiFunctionObject *func, AiObject *closure) {
    if (closure == NONE) {
        closure = NULL;
    }
    else if (closure && CHECK_TYPE_TUPLE(closure)) {
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

void function_dealloc(AiFunctionObject *func) {
    // ignore weakrefs yet
    DEC_REFCNT(func->func_code);
    DEC_REFCNT(func->func_globals);
    XDEC_REFCNT(func->func_module);
    DEC_REFCNT(func->func_name);
    XDEC_REFCNT(func->func_defaults);
    XDEC_REFCNT(func->func_doc);
    XDEC_REFCNT(func->func_dict);
    XDEC_REFCNT(func->func_closure);
    AiObject_GC_DEL(func);
}
