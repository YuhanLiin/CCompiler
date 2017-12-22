#include "test/utils/assert.h"
#include "ast/type.h"

#define signedOrNot(type, isSigned) assertEqNum(isSignedType(type), isSigned)
static void testSignedType(){
    signedOrNot(typInt8, 1);
    signedOrNot(typInt16, 1);
    signedOrNot(typInt32, 1);
    signedOrNot(typInt64, 1);

    signedOrNot(typUInt8, 0);
    signedOrNot(typUInt16, 0);
    signedOrNot(typUInt32, 0);
    signedOrNot(typUInt64, 0);
}

#define intOrFloat(type, isInt)do {\
    assertEqNum(isIntType(type), isInt);\
    assertEqNum(isFloatType(type), !isInt);\
} while(0);
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

#define checkArgPromotion(type, promoted) assertEqNum(argTypePromotion(type), promoted);
static void testArgPromotion(){
    checkArgPromotion(typFloat64, typFloat64);
    checkArgPromotion(typFloat32, typFloat64);
    checkArgPromotion(typInt64, typInt64);
    checkArgPromotion(typUInt64, typUInt64);
    checkArgPromotion(typUInt32, typUInt32);
    checkArgPromotion(typInt32, typInt32);
    checkArgPromotion(typUInt16, typInt32);
    checkArgPromotion(typInt16, typInt32);
    checkArgPromotion(typUInt8, typInt32);
    checkArgPromotion(typInt8, typInt32);
}

#define checkArithPromotion(type1, type2, promoted) do {\
    assertEqNum(arithTypePromotion(type1, type2), promoted);\
    assertEqNum(arithTypePromotion(type2, type1), promoted);\
} while (0)
static void testArithPromotion(){
    checkArithPromotion(typFloat64, typInt64, typFloat64);
    checkArithPromotion(typFloat64, typFloat32, typFloat64);
    checkArithPromotion(typUInt64, typFloat32, typFloat32);
    checkArithPromotion(typInt64, typUInt64, typUInt64);
    checkArithPromotion(typUInt64, typInt32, typUInt64);
    checkArithPromotion(typInt64, typInt32, typInt64);
    checkArithPromotion(typInt32, typUInt16, typInt32);
    checkArithPromotion(typInt32, typInt32, typInt32);
}

int main(int argc, char const *argv[])
{
    testSignedType();
    testArgPromotion();
    testArithPromotion();
    return 0;
}