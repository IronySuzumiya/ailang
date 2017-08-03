#pragma once
#ifndef CELL_OBJECT_H
#define CELL_OBJECT_H

#include "../system/utils.h"

typedef struct _cellobject {
    OBJECT_HEAD
    AiObject *ob_ref;
}
AiCellObject;

#define CHECK_TYPE_CELL(ob) CHECK_TYPE(ob, &type_cellobject)

#define CELL_GET(cell) (((AiCellObject *)(cell))->ob_ref)
#define CELL_SET(cell, ob) (CELL_GET(cell) = (ob))

AiAPI_DATA(AiTypeObject) type_cellobject;

AiAPI_FUNC(AiObject *) cell_new(AiObject *ob);
AiAPI_FUNC(AiObject *) cell_get(AiCellObject *cell);
AiAPI_FUNC(int) cell_set(AiCellObject *cell, AiObject *ob);

#endif
