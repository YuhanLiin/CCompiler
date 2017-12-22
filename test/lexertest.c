#include "array.h"
#include "lexer/lexer.h"
#include "utils.h"
#include <stdlib.h>

#include "test/utils/io.c"
#include "test/utils/assert.h"

Token token;

static void setup(const char_t* str){
    ioSetup(str);
    initLexer();
}

#define test(expectedTok) do {\
    assertEqNum(lexToken(), expectedTok);\
} while(0)

#define testStr(expectedTok, expectedStr) do {\
    test(expectedTok);\
    char_t* ___str = toCstring(&stringBuffer);\
    assertEqStr(___str, expectedStr);\
    free(___str);\
} while(0)

static void teardown(){
    disposeLexer();
}

//String literals may need to be replaced for different char types
static void testTokenIgnored(){
    setup("  \t/*adfsg 45tg*/ //\n\r /* / * */  //");
    test(tokEof);
    setup("/*");
    test(tokUnexpected);
    teardown();
}

static void testTokenKeywordIdentifier(){
    setup("ret return returning _DEF72  double long int float char short signed unsigned void while if else");
    testStr(tokIdent, "ret");
    test(tokReturn);
    testStr(tokIdent, "returning");
    testStr(tokIdent, "_DEF72");
    teardown();
    
    setup("long/float/char short signed serp shot sig unsigned void");
    test(tokLong);
    test(tokDiv);
    test(tokFloat);
    test(tokDiv);
    test(tokChar);
    test(tokShort);
    test(tokSigned);
    testStr(tokIdent, "serp");
    testStr(tokIdent, "shot");
    testStr(tokIdent, "sig");
    test(tokUnsigned);
    test(tokVoid);
    teardown();

    setup("while if int in else break continue");
    test(tokWhile);
    test(tokIf);
    test(tokInt);
    testStr(tokIdent, "in");
    test(tokElse);
    test(tokBreak);
    test(tokContinue);
    teardown();

    setup("d do doa doub double double0");
    testStr(tokIdent, "d");
    test(tokDo);
    testStr(tokIdent, "doa");
    testStr(tokIdent, "doub");
    test(tokDouble);
    testStr(tokIdent, "double0");
    teardown();
}

static void testTokenString(){
    setup("\"bobobobob \" \"\" \" a");
    testStr(tokString, "bobobobob ");
    testStr(tokString, "");
    test(tokUnexpected);
    //Unexpected tokens are not consumed
    assert(curChar == End);
    teardown();
}

static void testTokenChar(){
    setup("'");
    test(tokUnexpected);
    assert(curChar == End);
    teardown();
    setup("''");
    test(tokUnexpected);
    assert(curChar == End);
    teardown();
}

static void testTokenNumber(){
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

static void testTokenNumberExtensions(){
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

static void testTokenSymbols(){
    setup(", +++---*/! =+=-=*=/= ==<><=>=!= {};");
    test(tokComma);
    test(tokInc);
    test(tokPlus);
    test(tokDec);
    test(tokMinus);
    test(tokMulti);
    test(tokDiv);
    test(tokNot);
    
    test(tokAssign);
    test(tokPlusAssign);
    test(tokMinusAssign);
    test(tokMultiAssign);
    test(tokDivAssign);

    test(tokEquals);
    test(tokLess);
    test(tokGreater);
    test(tokLessEquals);
    test(tokGreaterEquals);
    test(tokNotEquals);

    test(tokLBrace);
    test(tokRBrace);
    test(tokSemicolon);
    test(tokEof);
    teardown();
}

static void testTokenEof(){
    setup("");
    test(tokEof);
    //EOF should be safe to parse multiple times
    test(tokEof);
}

//Makes sure all tokens except tokUnexpected are printable
static void testStringifyToken(){
    for (Token tok = tokEof; tok <= tokRBrace; tok++){
        stringifyToken(tok);
    }
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
    testTokenEof();
    testStringifyToken();
    return 0;
}