#pragma once
#include "utils.h"

typedef enum {
    typNone,    //Sentinel value for typeless situations
    typVoid,
    typInt8,
    typUInt8,
    typInt16,
    typUInt16,
    typInt32,
    typUInt32,
    typInt64,
    typUInt64,
    typFloat32,
    typFloat64
} Type;

const char_t* stringifyType(Type type);

Type argTypePromotion(Type type);
char checkTypeConvert(Type from, Type to);
Type arithTypePromotion(Type t1, Type t2);

char isSignedType(Type type);
char isIntType(Type type);
char isFloatType(Type type);