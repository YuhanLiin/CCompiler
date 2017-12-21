#include "utils.h"
#include "test/utils/assert.c"
#include <stdlib.h>

int driver(int argc, char_t const *argv[]);

#define FILE_COUNT 10
const char_t* CFILES[FILE_COUNT] = {
    "basic.c", "basicif.c", "binop.c", "params.c", 
    "unop.c", "void.c", "assign.c", "loop.c", 
    "controlflow.c", "condition.c"
};
const char_t* EXEFILES[FILE_COUNT] = {
    "basic.exe", "basicif.exe", "binop.exe", "params.exe", 
    "unop.exe", "void.exe", "assign.exe", "loop.exe", 
    "controlflow.exe", "condition.exe"
};
const int EXPECTED_OUT[FILE_COUNT] = {
    0, 1, 0, 3, 
    48, 6, 0, 42, 
    10, 17
};

static void testDriver(int i){
    const char_t *driverArgs[2];
    driverArgs[1] = CFILES[i];
    assertEqNum(driver(2, driverArgs), 0);
    assertEqNum(system(EXEFILES[i]), EXPECTED_OUT[i]);
}

int main(int argc, char const *argv[])
{
    DITCH_LEVEL = 1;
    for (int i=0; i<FILE_COUNT; i++){
        testDriver(i);
    }
    return 0;
}