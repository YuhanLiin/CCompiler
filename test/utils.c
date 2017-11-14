#include "lexer/lexer.h"
#include "utils.h"
#include "ast/type.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int DITCH_LEVEL = 0;

#define assertBegin() fprintf(stderr, "\nAssertion error in %s, line %d, in %s:\n", __FILE__, __LINE__, __func__)

#define assertFailHandler() do {\
    if (DITCH_LEVEL == 1) return;\
    if (DITCH_LEVEL == 2) exit(4);\
} while(0)

#define assertEqStr(actual, expected) do {\
    if (strcmp(expected, actual)){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be:\n%s\nbut got:\n%s\n",\
            #actual, expected, actual\
        );\
        assertFailHandler();\
    }\
} while(0);

#define assertEqNum(actual, expected) do {\
    if (expected != actual){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be %d, but got %d.\n",\
            #actual, expected, actual\
        );\
        assertFailHandler();\
    }\
} while(0);

#define assertEqFlt(actual, expected) do {\
    if (!doubleEq(expected, actual)){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be %.2f, but got %.2f.\n",\
            #actual, expected, actual\
        );\
    }\
} while(0);

char doubleEq(const double a, const double b){
    double diff = a - b;
    static double tolerance = 0.0001;
    return -tolerance < diff || diff < tolerance;
}
