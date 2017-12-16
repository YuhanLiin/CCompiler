#include "utils.h"
#include "test/utils/assert.c"
#include <stdlib.h>
int driver(int argc, char_t const *argv[]);

const char_t* CFILES[] = {"basic.c", "binop.c", "params.c", "unop.c", "void.c", "assign.c", "loop.c", "controlflow.c"};
const char_t* EXEFILES[] = {"basic.exe", "binop.exe", "params.exe", "unop.exe", "void.exe", "assign.exe", "loop.exe", "controlflow.exe"};
const int EXPECTED_OUT[] = {1, 46, 1, 48, 1, 8, 42, 10};
const int FILE_COUNT = 8;

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