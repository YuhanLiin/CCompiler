#include "./type.h"
#include "utils.h"
#include <assert.h>

const char_t* stringifyType(Type type){
    switch (type){
        case typInt32: return "int";
        case typInt64: return "long long";
        case typFloat32: return "float";
        case typFloat64: return "double";
        default:
            assert(0 && "Unprintable type, most likely typVoid or typNone");
    }
}