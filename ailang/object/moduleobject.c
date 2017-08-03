#include "../ailang.h"

AiTypeObject type_moduleobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "module",

};

AiObject *module_new(char *name) {
    AiModuleObject *m;
    AiObject *namestr;
    m = AiObject_GC_NEW(AiModuleObject);
    namestr = string_from_cstring(name);
    m->md_dict = dict_new();
    // TODO
    DEC_REFCNT(namestr);
    return (AiObject *)m;
}

AiObject *module_getdict(AiModuleObject *m) {
    if (m->md_dict) {
        return m->md_dict;
    }
    else {
        return m->md_dict = dict_new();
    }
}
