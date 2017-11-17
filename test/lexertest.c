#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include <stdlib.h>

#include "./io.c"
#include "./utils.c"

Token token;

void setup(const char_t* str){
    ioSetup(str);
    initLexer();
}
#define test(expectedTok) do {\
    assertEqNum(lexToken(), expectedTok);\
} while(0)
void teardown(){
    disposeLexer();
}

//String literals may need to be replaced for different char types
void testTokenIgnored(){
    setup("  \t/*adfsg 45tg*/ //\n\r /* / * */  //");
    test(tokEof);
    setup("/*");
    test(tokUnexpected);
    teardown();
}

void testTokenKeywordIdentifier(){
    setup("ret return returning _DEF72  double long int float char short signed unsigned shorts");
    test(tokIdent);
    assertEqStr(toCstring(&stringBuffer), "ret");
    test(tokReturn);
    test(tokIdent);
    assertEqStr(toCstring(&stringBuffer), "returning");
    test(tokIdent);
    assertEqStr(toCstring(&stringBuffer), "_DEF72");
    test(tokDouble);
    test(tokLong);
    test(tokInt);
    test(tokFloat);
    test(tokChar);
    test(tokShort);
    test(tokSigned);
    test(tokUnsigned);
    test(tokIdent);
    assertEqStr(toCstring(&stringBuffer), "shorts");
    test(tokEof);
    //Eof should be safe to parse multiple times
    test(tokEof);
    teardown();
}

void testTokenString(){
    setup("\"bobobobob \" \"\" \" a");
    test(tokString);
    assertEqStr(toCstring(&stringBuffer), "bobobobob ");
    test(tokString);
    assertEqStr(toCstring(&stringBuffer), "");
    test(tokUnexpected);
    //Unexpected tokens are not consumed
    assert(curChar == End);
    teardown();
}

void testTokenChar(){
    setup("'");
    test(tokUnexpected);
    assert(curChar == End);
    teardown();
    setup("''");
    test(tokUnexpected);
    assert(curChar == End);
    teardown();
}

void testTokenNumber(){
    setup("500(500.)60.54.7 '/' ..");
    test(tokNumInt);
    assertEqNum(intVal, 500);
    test(tokLParen);
    test(tokNumDouble);
    assertEqFlt(floatVal, 500);
    test(tokRParen);
    test(tokNumDouble);
    assertEqFlt(floatVal, 60.54);
    test(tokNumDouble);
    assertEqFlt(floatVal, .7);
    test(tokNumChar);
    assertEqNum(intVal, '/');
    test(tokUnexpected);
    assertEqNum(curChar, '.');
    teardown();
}

void testTokenNumberExtensions(){
    setup("500L 500l 500Ll 500u 500lU 500LLU 5.5f 5.5F");
    test(tokNumInt);
    test(tokNumInt);
    test(tokNumLong);
    test(tokNumUInt);
    test(tokNumUInt);
    test(tokNumULong);
    test(tokNumFloat);
    test(tokNumFloat);
    teardown();
}

void testTokenSymbols(){
    setup(",+-*/{};");
    test(tokComma);
    test(tokPlus);
    test(tokMinus);
    test(tokMulti);
    test(tokDiv);
    test(tokLBrace);
    test(tokRBrace);
    test(tokSemicolon);
    test(tokEof);
    teardown();
}

int main(int argc, char const *argv[])
{
    DITCH_LEVEL = 1;
    testTokenIgnored();
    testTokenSymbols();
    testTokenChar();
    testTokenNumber();
    testTokenNumberExtensions();
    testTokenString();
    testTokenKeywordIdentifier();
    return 0;
}