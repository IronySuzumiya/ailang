#pragma once
#ifndef CELL_OBJECT_H
#define CELL_OBJECT_H

#include "../aiconfig.h"

typedef struct _cellobject {
    AiObject_HEAD
    AiObject *ob_ref;
}
AiCellObject;

#define CHECK_TYPE_CELL(ob) CHECK_TYPE(ob, &AiType_Cell)

#define CELL_GET(cell) (((AiCellObject *)(cell))->ob_ref)
#define CELL_SET(cell, ob) (CELL_GET(cell) = (ob))

AiAPI_DATA(AiTypeObject) AiType_Cell;

AiAPI_FUNC(AiObject *) AiCell_New(AiObject *ob);
AiAPI_FUNC(AiObject *) AiCell_Get(AiCellObject *cell);
AiAPI_FUNC(int) AiCell_Set(AiCellObject *cell, AiObject *ob);

#endif
