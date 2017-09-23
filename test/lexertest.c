#include "../array.h"
#include "../lexer.h"
#include "../utils.h"
#include <stdlib.h>

#include "io.c"
#include "utils.c"

Token token;

#define setup(str) do {ioSetup(str); initLexer();} while(0)
#define test(expectedTok) do {token = lexToken(); assert(token == expectedTok);} while(0)

//String literals may need to be replaced for different char types
void testTokenIgnored(){
    setup("  \t/*adfsg 45tg*/ //\n\r /* / * */  //");
    test(tokEof);
    setup("/*");
    test(tokUnexpected);
}

void testTokenKeywordIdentifier(){
    setup("ret return returning _DEF72  double long int float l ");
    test(tokIdent);
    assert(strEq(&stringBuffer, "ret"));
    test(tokReturn);
    test(tokIdent);
    assert(strEq(&stringBuffer, "returning"));
    test(tokIdent);
    assert(strEq(&stringBuffer, "_DEF72"));
    test(tokDouble);
    test(tokLong);
    test(tokInt);
    test(tokFloat);
    test(tokIdent);
    assert(strEq(&stringBuffer, "l"));
    test(tokEof);
}

void testTokenString(){
    setup("\"bobobobob \" \"\" \" a");
    test(tokString);
    assert(strEq(&stringBuffer, "bobobobob "));
    test(tokString);
    assert(strEq(&stringBuffer, ""));
    test(tokUnexpected);
    assert(curChar == End);
}

void testTokenNumber(){
    setup("500(500.)60.54.7 ..");
    test(tokNumInt);
    assert(intVal == 500);
    test(tokLParen);
    test(tokNumDouble);
    assert(doubleEq(floatVal, 500));
    test(tokRParen);
    test(tokNumDouble);
    assert(doubleEq(floatVal, 60.54));
    test(tokNumDouble);
    assert(doubleEq(floatVal, .7));
    test(tokUnexpected);
    assert(curChar == '.');
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
}



int main(int argc, char const *argv[])
{
    testTokenIgnored();
    testTokenSymbols();
    testTokenNumber();
    testTokenString();
    testTokenKeywordIdentifier();
    return 0;
}