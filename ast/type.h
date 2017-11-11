#pragma once
#include "utils.h"

typedef enum {
    typNone,    //Sentinel value for typeless situations
    typVoid,
    typInt32,
    typInt64,
    typFloat32,
    typFloat64
} Type;

const char_t* stringifyType(Type type);