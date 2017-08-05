#include "../ailang.h"

AiTypeObject AiType_Module = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "module",

};

AiObject *AiModule_New(char *name) {
    AiModuleObject *m;
    AiObject *namestr;
    m = AiMem_Alloc(sizeof(AiModuleObject));
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
