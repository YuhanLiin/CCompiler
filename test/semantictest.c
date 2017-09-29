#include "../semantic.h"
#include "../utils.h"
#include "../ast.h"
#include "../parser.h"

#include "io.c"
#include "utils.c"

void test(char (*verifier)(Ast*), const char_t* inputStr, char expectedResult) {
    ioSetup(inputStr);
    initLexer();
    initParser();
    assert(verifier(parseTopLevel()) == expectedResult);
    disposeLexer();
}

void testVerifyMain(){
    test(verifyTopLevel, "int main() return 4;", 1);
    //These 2 will change in future
    test(verifyTopLevel, "int main() return 4.5;", 0);
    test(verifyTopLevel, "float main() return 44.4;", 1);
    test(verifyTopLevel, "int notmain() return 4;", 0);
}

int main(int argc, char const *argv[])
{
    testVerifyMain();
    return 0;
}