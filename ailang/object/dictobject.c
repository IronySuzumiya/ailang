#include "../ailang.h"

TypeObject type_dictobject = {
    INIT_OBJECT_HEAD(&type_typeobject)
};

Object *dict_new() {
    return NULL;
}

Object *dict_getitem(Object *dict, Object *key) {
    return NULL;
}

void dict_setitem(Object *dict, Object *key, Object *val) {

}

int dict_delitem(Object *dict, Object *key) {
    return 0;
}
