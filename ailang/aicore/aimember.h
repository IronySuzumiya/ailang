#pragma once
#ifndef MEMBER_H
#define MEMBER_H

#include "../aiconfig.h"

typedef struct _memberdef {
    char *name;
    int type;
    ssize_t offset;
    int flags;
}
AiMemberDef;

#endif
