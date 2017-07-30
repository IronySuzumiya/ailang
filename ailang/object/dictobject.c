#include "../ailang.h"

AiTypeObject type_dictobject = {
    INIT_OBJECT_HEAD(&type_typeobject)
};

AiObject *dict_new() {
    return NULL;
}

AiObject *dict_getitem(AiObject *dict, AiObject *key) {
    return NULL;
}

void dict_setitem(AiObject *dict, AiObject *key, AiObject *val) {

}

int dict_delitem(AiObject *dict, AiObject *key) {
    return 0;
}
