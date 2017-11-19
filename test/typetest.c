#include "./utils.c"
#include "ast/type.h"

static void testSignedType(){
    assertEqNum(isSignedType(typInt8), 1);
    assertEqNum(isSignedType(typInt16), 1);
    assertEqNum(isSignedType(typInt32), 1);
    assertEqNum(isSignedType(typInt64), 1);

    assertEqNum(isSignedType(typUInt8), 0);
    assertEqNum(isSignedType(typUInt16), 0);
    assertEqNum(isSignedType(typUInt32), 0);
    assertEqNum(isSignedType(typUInt64), 0);
}

static void intOrFloat(Type type, char isInt){
    assertEqNum(isIntType(type), isInt);
    assertEqNum(isFloatType(type), !isInt);
}
static void testIsIntOrFloat(){
    intOrFloat(typInt8, 1);
    intOrFloat(typInt16, 1);
    intOrFloat(typInt32, 1);
    intOrFloat(typInt64, 1);
    intOrFloat(typUInt8, 1);
    intOrFloat(typUInt16, 1);
    intOrFloat(typUInt32, 1);
    intOrFloat(typUInt64, 1);
    intOrFloat(typFloat32, 0);
    intOrFloat(typFloat64, 0);
}

static void testArgPromotion(){
    assertEqNum(argTypePromotion(typFloat64), typFloat64);
    assertEqNum(argTypePromotion(typFloat32), typFloat64);
    assertEqNum(argTypePromotion(typUInt64), typUInt64);
    assertEqNum(argTypePromotion(typInt64), typInt64);
    assertEqNum(argTypePromotion(typUInt32), typInt64);
    assertEqNum(argTypePromotion(typInt32), typInt64);
    assertEqNum(argTypePromotion(typUInt16), typInt64);
    assertEqNum(argTypePromotion(typInt16), typInt64);
    assertEqNum(argTypePromotion(typUInt8), typInt64);
    assertEqNum(argTypePromotion(typInt8), typInt64);
}

static void testArithPromotion(){
    assertEqNum(arithTypePromotion(typFloat64, typInt64), typFloat64);
    assertEqNum(arithTypePromotion(typFloat64, typFloat32), typFloat64);
    assertEqNum(arithTypePromotion(typUInt64, typFloat32), typFloat32);
    assertEqNum(arithTypePromotion(typInt64, typUInt64), typUInt64);
    assertEqNum(arithTypePromotion(typUInt64, typInt32), typUInt64);
    assertEqNum(arithTypePromotion(typInt32, typUInt16), typInt64);
    assertEqNum(arithTypePromotion(typInt32, typInt32), typInt64);
}

int main(int argc, char const *argv[])
{
    testSignedType();
    testArgPromotion();
    testArithPromotion();
    return 0;
}