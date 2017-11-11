#include "lexer/lexer.h"
#include "utils.h"
#include "ast/type.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define assertBegin() fprintf(stderr, "Assertion error in %s, line %d, in %s:\n", __FILE__, __LINE__, __func__)

#define assertEqStr(actual, expected) do {\
    if (strcmp(expected, actual)){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be:\n%s\nbut got:\n%s\n",\
            #actual, expected, actual\
        );\
    }\
} while(0);

#define assertEqNum(actual, expected) do {\
    if (expected != actual){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be %.2Lf, but got %.2Lf.\n",\
            #actual, (expected)*1.0, (actual)*1.0\
        );\
    }\
} while(0);

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