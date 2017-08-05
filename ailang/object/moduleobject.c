#include "../ailang.h"

AiTypeObject AiType_Module = {
    INIT_AiVarObject_HEAD(&AiType_Type, 0)
    "module",

};

AiObject *AiModule_New(char *name) {
    AiModuleObject *m;
    AiObject *namestr;
    m = AiObject_GC_New(AiModuleObject);
    namestr = AiString_From_String(name);
    m->md_dict = AiDict_New();
    // TODO
    DEC_REFCNT(namestr);
    return (AiObject *)m;
}

AiObject *AiModule_Getdict(AiModuleObject *m) {
    if (m->md_dict) {
        return m->md_dict;
    }
    else {
        return m->md_dict = AiDict_New();
    }
}
