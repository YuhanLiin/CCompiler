#include "utils.h"
#include "test/utils.c"
#include <stdlib.h>
int driver(int argc, char_t const *argv[]);

const char_t* CFILES[] = {"basic.c", "binop.c", "params.c", "unop.c", "void.c"};
const char_t* EXEFILES[] = {"basic.exe", "binop.exe", "params.exe", "unop.exe", "void.exe"};
const int EXPECTED_OUT[] = {1, 46, 1, 3, 1};
const int FILE_COUNT = 5;

static void testDriver(int i){
    DITCH_LEVEL = 1;
    const char_t *driverArgs[2];
    driverArgs[1] = CFILES[i];
    assertEqNum(driver(2, driverArgs), 0);
    assertEqNum(system(EXEFILES[i]), EXPECTED_OUT[i]);
}

int main(int argc, char const *argv[])
{
    for (int i=0; i<FILE_COUNT; i++){
        testDriver(i);
    }
    return 0;
}