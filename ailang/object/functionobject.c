#include "../ailang.h"

AiTypeObject type_functionobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)

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
