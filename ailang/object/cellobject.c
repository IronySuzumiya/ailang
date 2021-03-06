#include "../ailang.h"

static void cell_dealloc(AiCellObject *cell);
static int cell_compare(AiCellObject *lhs, AiCellObject *rhs);

AiTypeObject AiType_Cell = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "cell",                             /* tp_name */
    sizeof(AiCellObject),               /* tp_basesize */
    0,                                  /* tp_itemsize */

    (destructor)cell_dealloc,           /* tp_dealloc */
    (cmpfunc)cell_compare,              /* tp_compare */
};

AiObject *AiCell_New(AiObject *ob) {
    AiCellObject *cell;

    cell = AiObject_NEW(AiCellObject, &AiType_Cell);
    cell->ob_ref = ob;
    XINC_REFCNT(ob);

    return (AiObject *)cell;
}

AiObject *AiCell_Get(AiCellObject *cell) {
    if (!CHECK_TYPE_CELL(cell)) {
        FATAL_ERROR("bad cell visit");
        return NULL;
    }
    XINC_REFCNT(cell->ob_ref);
    return CELL_GET(cell);
}

int AiCell_Set(AiCellObject *cell, AiObject *ob) {
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
    AiObject_Del(cell);
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
