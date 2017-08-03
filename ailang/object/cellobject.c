#include "../ailang.h"

static void cell_dealloc(AiCellObject *cell);
static int cell_compare(AiCellObject *lhs, AiCellObject *rhs);
static void cell_free(void *p);

AiTypeObject type_cellobject = {
    INIT_OBJECT_VAR_HEAD(&type_typeobject, 0)
    "cell",
    (destructor)cell_dealloc,
    0,
    (cmpfunc)cell_compare,

    0,
    0,
    0,

    0,
    0,
    (freefunc)cell_free,
};

AiObject *cell_new(AiObject *ob) {
    AiCellObject *cell;

    cell = (AiCellObject *)AiObject_GC_NEW(AiCellObject);
    INIT_OBJECT(cell, &type_cellobject);
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
    OB_FREE(cell);
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

void cell_free(void *p) {
    AiMEM_FREE(p);
}
