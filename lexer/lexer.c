#include "array.h"
#include "./lexer.h"
#include "utils.h"
#include <assert.h>
#include <stdint.h>

char_t curChar; //Updated for every character consumed
//Line number and position at end of every token. Actively updated by lexer
size_t lineNumber;
size_t linePos;
size_t lineNumberTokStart;
size_t linePosTokStart;
Array(char_t) stringBuffer; //Store identifier and strings
double floatVal;   //Store number tokens
uint64_t intVal;

//Character matchers
static char isSpace(char_t c){
    return c ==' ' || c =='\t';
}
static char isEol(char_t c){ //End of line chars
    return c=='\n' || c=='\r';
}
static char isAlpha(char_t c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static char isDigit(char_t c){
    return c >= '0' && c <= '9';
}
static char isIdentChar(char_t c){
    return isAlpha(c) || isDigit(c) || c == '_' || c == '$';
}

//Consumes char and updates curChar. Called by tokenizer
static char_t getNext(){
    //Increases line number or position
    if (isEol(curChar)){
        lineNumber++;
        linePos = 0;
    }
    else linePos++;
    curChar = consumeNext();
    return curChar;
}
//Store in string
static char_t store(char_t c){
    if (!arrPush(char_t)(&stringBuffer, c)) exit(1);
    return c;
}
//Get next char if it matches c. REturns whether match occurs
static char getNextIf(char_t c){
    if (curChar == c){
        getNext();
        return 1;
    }
    return 0;
}
static void storeNext(char_t c){
    store(c);
    getNext();
}
//Get and store next char if it matches c. REturns whether match occurs
static char storeNextIf(char_t c){
    if (getNextIf(c)){
        store(c);
        return 1;
    }
    return 0;
}

void initLexer(){ //Can fail due to malloc
    if (!arrInit(char_t)(&stringBuffer, 5, NULL, NULL)) exit(1); //Allocate empty string
    linePos = 0;
    lineNumber = 1;
    linePosTokStart = 0;
    lineNumberTokStart = 1;
    curChar = consumeNext();
}

void disposeLexer(){
    arrDispose(char_t)(&stringBuffer);
}

//Recognizes decimal sequence and adds it to floatVal. Returns whether decimals were recognized
static char lexDecimals(){
    if (isDigit(curChar)){
        double d = 0.1;
        do{
            floatVal += d * (curChar - '0');
            getNext();
            d /= 10;
        } while(isDigit(curChar));
        return 1;
    }
    return 0;
}

static char lexKeywordPart(const char_t *keyword){
    while (*keyword != '\0'){
        if (!storeNextIf(*keyword)){
            return 0;
        }
        keyword++;
    }
    return 1;
}
//In addition to matching keyword, this makes sure that the keyword isn't followed by identifier chars
static char lexKeyword(const char_t *keyword){
    if (lexKeywordPart(keyword) && !isIdentChar(curChar)){
        return 1;
    }
    return 0;
}

//Gets the next token
Token lexToken(){
    //Reset fields
    arrClear(char_t)(&stringBuffer);
    floatVal = 0;
    intVal = 0;

    begin:
    lineNumberTokStart = lineNumber;
    linePosTokStart = linePos;
    //Skip over newlines, whitespace
    if (isSpace(curChar) || isEol(curChar)){
        getNext();
        goto begin;
    }
     
    switch (curChar) {
        case 'b':
            //break
            if (lexKeyword("break")){
                return tokBreak;
            }
            goto identifier;
        case 'c':
            //char
            storeNext('c');
            if (curChar == 'h'){
                if (lexKeyword("har")){
                    return tokChar;
                }
            }
            else if (lexKeyword("ontinue")){
                return tokContinue;
            }
            goto identifier;
        case 'd':
            //double or do
            if (lexKeywordPart("do")){
                //If "do" is not followed by identifier char that makes it an isolated keyword
                if (!isIdentChar(curChar)){
                    return tokDo;
                }
                //If do is followed directly by "uble" keyword then it's the keyword double
                if (lexKeyword("uble")){
                    return tokDouble;
                }
            }
            goto identifier;
        case 'e':
            //else
            if (lexKeyword("else")){
                return tokElse;
            }
            goto identifier;
        case 'f':
            //float keyword
            if (lexKeyword("float")){
                return tokFloat;
            } 
            goto identifier;
        case 'i':
            storeNext('i');
            //int or if keyword
            if (curChar == 'n'){
                if (lexKeyword("nt")){
                    return tokInt;
                }
            }
            else if (lexKeyword("f")){
                return tokIf;
            }
            goto identifier;
        case 'l':
            //long keyword
            if (lexKeyword("long")){
                return tokLong;
            } 
            goto identifier;
        case 'r':
            //Return keyword
            if (lexKeyword("return")){
                return tokReturn;
            } 
            goto identifier;
        case 's':
            //signed or short
            storeNext('s');
            if (curChar == 'i'){
                if (lexKeyword("igned")){
                    return tokSigned;
                }
            }
            else if (lexKeyword("hort")){
                return tokShort;
            }
            goto identifier;
        case 'u':
            //unsigned
            if (lexKeyword("unsigned")){
                return tokUnsigned;
            }
            goto identifier;
        case 'v':
            //void
            if (lexKeyword("void")){
                return tokVoid;
            }
            goto identifier; 
        case 'w':
            //while
            if (lexKeyword("while")){
                return tokWhile;
            }
            goto identifier;
        

        //Match string "[anychar]"
        case '"':
            getNext();
            while (curChar != '"'){
                //If file ends in middle of a string return token for unexpected char
                if (curChar == End){
                    return tokUnexpected;
                }
                storeNext(curChar);
            }
            getNext();
            return tokString;
        case '\'':
            getNext();
            if (curChar == End || isEol(curChar)){
                return tokUnexpected;
            }
            intVal = (unsigned char)curChar;
            getNext();
            if (curChar != '\''){
                return tokUnexpected;
            }
            getNext();
            return tokNumChar;
        case '=':
            getNext();
            if (curChar == '='){
                getNext();
                return tokEquals;
            }
            return tokAssign;
        case '!':
            getNext();
            if (curChar == '='){
                getNext();
                return tokNotEquals;
            }
            return tokNot;
        case '>':
            getNext();
            if (curChar == '='){
                getNext();
                return tokGreaterEquals;
            }
            return tokGreater;
        case '<':
            getNext();
            if (curChar == '='){
                getNext();
                return tokLessEquals;
            }
            return tokLess;
        case '+':
            getNext();
            if (curChar == '+'){
                getNext();
                return tokInc;
            }
            else if (curChar == '='){
                getNext();
                return tokPlusAssign;
            }
            return tokPlus;
        case '-':
            getNext();
            if (curChar == '-'){
                getNext();
                return tokDec;
            }
            else if (curChar == '='){
                getNext();
                return tokMinusAssign;
            }
            return tokMinus;
        case '*':
            getNext();
            if (curChar == '='){
                getNext();
                return tokMultiAssign;
            }
            return tokMulti;
        case ',':
            getNext();
            return tokComma;
        case ';':
            getNext();
            return tokSemicolon;
        case '{':
            getNext();
            return tokLBrace;
        case '}':
            getNext();
            return tokRBrace;
        case '(':
            getNext();
            return tokLParen;
        case ')':
            getNext();
            return tokRParen;
        case '/':
            getNext();
            if (curChar == '='){
                getNext();
                return tokDivAssign;
            }
            //Single line comment
            else if (curChar == '/'){
                getNext();
                //Comment ends when end of line or end of file is seen
                while (!isEol(curChar) && curChar != End){
                    getNext();
                }
                goto begin; //No token to return, so start lexing again
            }
            //Multi line comment
            else if (curChar == '*'){
                getNext();  //Consume *
                char_t prev;
                do {
                    if (curChar == End){
                        return tokUnexpected;
                    }
                    prev = curChar;
                    getNext();
                  //If the last consumed comment char is * and the one after it is /, then comment is over
                } while (prev != '*' || curChar != '/');
                getNext(); //Consume /
                goto begin; //No token to return, so start lexing again
            }
            return tokDiv;
        case '.':
            //double .[0-9]+(f|F)?
            getNext(); 
            if (lexDecimals()){
                if (curChar == 'f' || curChar == 'F'){
                    getNext();
                    return tokNumFloat;
                }
                return tokNumDouble;
            }
            else{
                return tokUnexpected; //Decimal sequence must exist after dot, or syntax error
            }
        case End:
            return tokEof;
        default:
            //Number [0-9]+(.[0-9]*)?
            if (isDigit(curChar)){
                do{ //Convert first set of digits as integer
                    intVal *= 10;
                    intVal += curChar - '0';
                    getNext();
                }while(isDigit(curChar));
                //If dot follows, attempt to turn next digits into decimals and return as double
                if (getNextIf('.')){
                    floatVal = intVal;
                    lexDecimals();
                    //Regardless of whether decimals can be parsed, still return double/float
                    if (curChar == 'f' || curChar == 'F'){
                        getNext();
                        return tokNumFloat;
                    }
                    return tokNumDouble;
                }
                if (curChar == 'l' || curChar == 'L'){
                    getNext();
                    if (curChar == 'l' || curChar == 'L'){
                        getNext();
                        if (curChar == 'u' || curChar == 'U'){
                            getNext();
                            return tokNumULong;
                        }
                        return tokNumLong;
                    }
                }
                if (curChar == 'u' || curChar == 'U'){
                    getNext();
                    return tokNumUInt;
                }
                return tokNumInt;
            }
            //Identifier [a-zA-Z][a-zA-Z_0-9]*
            else if (isIdentChar(curChar)){
                identifier:
                //Continue if alphanumeric match doesnt end at keyword or no keyword exists and return identifier
                while(isIdentChar(curChar)) {
                    storeNext(curChar);
                }
                return tokIdent;
            }
            //Syntax error otherwise
            return tokUnexpected;
    } 
    assert(0 && "Lexer missing case");
}

char isAssignmentOp(Token op){
    switch (op){
        case tokPlusAssign:
        case tokMinusAssign:
        case tokMultiAssign:
        case tokDivAssign:
        case tokAssign:
            return 1;
    }
    return 0;
}

const char_t * stringifyToken(Token tok){
    switch(tok){
        case tokEof:
            return "end of file";
        case tokReturn:
            return "keyword \"return\"";
        case tokVoid:
            return "keyword \"void\"";
        case tokInt:
            return "keyword \"int\"";
        case tokLong:
            return "keyword \"long\"";
        case tokFloat:
            return "keyword \"float\""; 
        case tokDouble:
            return "keyword \"double\"";
        case tokUnsigned:
            return "keyword \"unsigned\"";
        case tokSigned:
            return "keyword \"signed\"";
        case tokChar:
            return "keyword \"char\"";
        case tokShort:
            return "keyword \"short\"";
        case tokIf:
            return "keyword \"if\"";
        case tokElse:
            return "keyword \"else\"";
        case tokWhile:
            return "keyword \"while\"";
        case tokDo:
            return "keyword \"do\"";
        case tokBreak:
            return "keyword \"break\"";
        case tokContinue:
            return "keyword \"continue\"";
        case tokIdent:
            return "identifier";
        case tokNumDouble:
        case tokNumFloat:
            return "floating-point number";
        case tokNumLong:
        case tokNumInt:
        case tokNumULong:
        case tokNumUInt:
            return "integer literal";
        case tokNumChar:
            return "character literal";
        case tokString:
            return "string literal";
        case tokNot:
            return "!";
        case tokPlus:
            return "+";
        case tokMinus:
            return "-";
        case tokInc:
            return "++";
        case tokDec:
            return "--";
        case tokAssign:
            return "=";
        case tokPlusAssign:
            return "+=";
        case tokMinusAssign:
            return "-=";
        case tokMultiAssign:
            return "*=";
        case tokDivAssign:
            return "/=";
        case tokEquals:
            return "==";
        case tokNotEquals:
            return "!=";
        case tokGreater:
            return ">";
        case tokLess:
            return "<";
        case tokGreaterEquals:
            return ">=";
        case tokLessEquals:
            return "<=";
        case tokDiv:
            return "/";
        case tokMulti:
            return "*";
        case tokComma:
            return ",";
        case tokLParen:
            return "(";
        case tokRParen:
            return ")";
        case tokSemicolon:
            return ";";
        case tokLBrace:
            return "{";
        case tokRBrace:
            return "}";
        default:
            assert(0 && "Unhandled token");
    }     
}