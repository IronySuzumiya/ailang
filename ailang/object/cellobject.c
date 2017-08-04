#include "../ailang.h"

static void cell_dealloc(AiCellObject *cell);
static int cell_compare(AiCellObject *lhs, AiCellObject *rhs);

AiTypeObject type_cellobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "cell",                             /* tp_name */
    sizeof(AiCellObject),               /* tp_basesize */
    0,                                  /* tp_itemsize */
    (destructor)cell_dealloc,           /* tp_dealloc */
    0,                                  /* tp_print */
    (cmpfunc)cell_compare,              /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//object_generic_getattr,             /* tp_getattro */
    0,                                  /* tp_setattro */

    0,                                  /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,//cell_getsetlist,                    /* tp_getset */
};

AiObject *cell_new(AiObject *ob) {
    AiCellObject *cell;

    cell = (AiCellObject *)AiObject_GC_NEW(AiCellObject);
    INIT_AiObject(cell, &type_cellobject);
    cell->ob_ref = ob;
    XINC_REFCNT(ob);

    return (AiObject *)cell;
}

AiObject *cell_get(AiCellObject *cell) {
    if (!CHECK_TYPE_CELL(cell)) {
        FATAL_ERROR("bad cell visit");
        return NULL;
    }
    XINC_REFCNT(cell->ob_ref);
    return CELL_GET(cell);
}

int cell_set(AiCellObject *cell, AiObject *ob) {
    if (!CHECK_TYPE_CELL(cell)) {
        FATAL_ERROR("bad cell visit");
        return -1;
    }
    XDEC_REFCNT(CELL_GET(cell));
    XINC_REFCNT(ob);
    CELL_SET(cell, ob);
    return 0;
}

void cell_dealloc(AiCellObject *cell) {
    XDEC_REFCNT(CELL_GET(cell));
    AiObject_GC_DEL(cell);
}

int cell_compare(AiCellObject *lhs, AiCellObject *rhs) {
    if (!CELL_GET(lhs)) {
        if (!CELL_GET(rhs)) {
            return 0;
        }
        else {
            return -1;
        }
    }
    else if(!CELL_GET(rhs)) {
        return 1;
    }
    else {
        return CELL_GET(lhs)->ob_type->tp_compare((AiObject *)lhs, (AiObject *)rhs);
    }
}
