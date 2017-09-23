#include "../lexer.h"
#include <assert.h>
#include "../semantic.h"

char strEq(Array(char_t)* string, const char_t * cstr){
    for (size_t i=0; i<string->size; i++){
        if (cstr[i] == '\0' || cstr[i] != string->elem[i]){
            return 0;
        }
    }
    return 1;
}

char doubleEq(const double a, const double b){
    double diff = a - b;
    static double tolerance = 0.0001;
    return -tolerance < diff || diff < tolerance;
}

const char_t *stringifyOp(Token tok){
    switch(tok){
        case tokPlus:
            return "+";
        case tokMinus:
            return "-";
        case tokDiv:
            return "/";
        case tokMulti:
            return "*";
        default:
            assert (0 && "Not an op");
    }
}

const char_t *stringifyType(Type type){
    switch(type){
        case typInt32:
            return "i32";
        case typInt64:
            return "i64";
        case typFloat32:
            return "f32";
        case typFloat64:
            return "f64";
        default:
            assert(0 && "Invalid type");
    }
}