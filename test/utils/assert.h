#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define DITCH_LEVEL 0

#define assertBegin() do{\
    fprintf(stderr, "\nAssertion error in %s, line %d, in %s:\n", __FILE__, __LINE__, __func__);\
} while(0)

#define assertFailHandler() do {\
    if (DITCH_LEVEL == 1) return;\
    if (DITCH_LEVEL == 2) exit(4);\
} while(0)

#define assertEqStr(actual, expected) do {\
    const char_t* __actual = actual;\
    const char_t* __expected = expected;\
    if (strcmp(__expected, __actual)){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be:\n%s\nbut got:\n%s\n",\
            #actual, __expected, __actual\
        );\
        assertFailHandler();\
    }\
} while(0);

#define assertEqNum(actual, expected) do {\
    uint64_t __actual = (uint64_t)actual;\
    uint64_t __expected = (uint64_t)expected;\
    if (__expected != __actual){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be %d, but got %d.\n",\
            #actual, __expected, __actual\
        );\
        assertFailHandler();\
    }\
} while(0);

#define assertNotEqNum(actual, notEqual) do {\
    uint64_t __actual = (uint64_t)actual;\
    uint64_t __notEqual = (uint64_t)notEqual;\
    if (__notEqual == __actual){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to NOT equal %d\n",\
            #actual, __notEqual\
        );\
        assertFailHandler();\
    }\
} while(0);

#define assertEqFlt(actual, expected) do {\
    double __actual = actual;\
    double __expected = expected;\
    if (!doubleEq(__expected, __actual)){\
        assertBegin();\
        fprintf(\
            stderr, "Expected %s to be %.2f, but got %.2f.\n",\
            #actual, __expected, __actual\
        );\
        assertFailHandler();\
    }\
} while(0);

// Will evaluate a and b multiple times, so must be used with care
#define doubleEq(a, b) -0.0001 < ((a)-(b)) || ((a)-(b)) < 0.0001