#include "./type.h"
#include "utils.h"
#include <assert.h>

const char_t* stringifyType(Type type){
    switch (type){
        case typVoid: return "void";
        case typUInt8: return "unsigned char";
        case typInt8: return "char";
        case typUInt16: return "unsigned short";
        case typInt16: return "short";
        case typUInt32: return "unsigned int";
        case typInt32: return "int";
        case typUInt64: return "unsigned long long";
        case typInt64: return "long long";
        case typFloat32: return "float";
        case typFloat64: return "double";
        default:
            assert(0 && "Unprintable type, most likely typNone");
    }
}

char isSignedType(Type type){
    switch (type){
        case typUInt8: return 0;
        case typInt8: return 1;
        case typUInt16: return 0;
        case typInt16: return 1;
        case typUInt32: return 0;
        case typInt32: return 1;
        case typUInt64: return 0;
        case typInt64: return 1;
        case typFloat32: return 1;
        case typFloat64: return 1;
        default:
            assert(0 && "Not a type for which signedness matters");
    }
}

Type intTypePromotion(Type type){
    switch (type){
        case typUInt32:
            return typUInt32;
        case typUInt8:
        case typUInt16:
        case typInt8:
        case typInt16:
        case typInt32:
            return typInt32;
        default:
            assert(0 && "Type invalid for int promotion");
    }
}

//TODO actual conversion checks
char checkTypeConvert(Type from, Type to){
    return from == to;
}

Type arithTypePromotion(Type t1, Type t2){
    if (t1 == typFloat64 || t2 == typFloat64){
        return typFloat64;
    }
    if (t1 == typFloat32 || t2 == typFloat32){
        return typFloat32;
    }
    if (t1 == typUInt64 || t2 == typUInt64){
        return typUInt64;
    }
    if (t1 == typInt64 || t2 == typInt64){
        return typInt64;
    }
    t1 = intTypePromotion(t1);
    t2 = intTypePromotion(t2);
    if (t1 == typUInt32 || t2 == typUInt32){
        return typUInt32;
    }
    return typInt32;
}