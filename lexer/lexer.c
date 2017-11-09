#include "array.h"
#include "./lexer.h"
#include "utils.h"
#include <assert.h>

char_t curChar; //Updated for every character consumed
//Line number and position at start of every token. Used for errors
size_t lineNumber = 1;
size_t linePos;
//Line number and position at end of every token. Actively updated by lexer
size_t lineTokenEndNumber;
size_t lineTokenEndPos;
Array(char_t) stringBuffer; //Store identifier and strings
double floatVal;   //Store number tokens
long long intVal;

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
        lineTokenEndNumber++;
        lineTokenEndPos = 0;
    }
    else lineTokenEndPos++;
    curChar = consumeNext();
    return curChar;
}
//Store in string
static char_t store(char_t c){
    arrPush(char_t)(&stringBuffer, c);
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
    lineTokenEndPos = 0;
    lineTokenEndNumber = 1;
    lineNumber = 1;
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

static char lexKeyword(const char_t *keyword){
    while (*keyword != '\0'){
        if (!storeNextIf(*keyword)){
            return 0;
        }
        keyword++;
    }
    if (isIdentChar(curChar)){
        return 0;
    }
    return 1;
}

//Gets the next token
Token lexToken(){
    //Reset fields
    arrClear(char_t)(&stringBuffer);
    floatVal = 0;
    intVal = 0;

    begin:
    //Update line number and position to start of new token
    lineNumber = lineTokenEndNumber;
    linePos = lineTokenEndPos;

    //Skip over newlines, whitespace
    if (isSpace(curChar) || isEol(curChar)){
        getNext();
        goto begin;
    }
     
    switch (curChar) {
        case 'r':
            //Return keyword
            if (lexKeyword("return")){
                return tokReturn;
            } 
            goto identifier;
        case 'i':
            //int keyword
            if (lexKeyword("int")){
                return tokInt;
            } 
            goto identifier;
        case 'l':
            //long keyword
            if (lexKeyword("long")){
                return tokLong;
            } 
            goto identifier;
        case 'f':
            //float keyword
            if (lexKeyword("float")){
                return tokFloat;
            } 
            goto identifier;
        case 'd':
            //double keyword
            if (lexKeyword("double")){
                return tokDouble;
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
                store(curChar);
                getNext();
            }
            getNext();
            return tokString;
        case '+':
            getNext();
            return tokPlus;
        case '-':
            getNext();
            return tokMinus;
        case '*':
            getNext();
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
            //Single line comment
            if (curChar == '/'){
                getNext();
                //Comment ends when end of line of end of file is seen
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
            //double .[0-9]+
            getNext(); 
            if (lexDecimals()){
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
                    //Regardless of whether decimals can be parsed, still return double
                    return tokNumDouble;
                }
                return tokNumInt;  //If no dot then it's int
            }
            //Identifier [a-zA-Z][a-zA-Z_0-9]*
            else if (isIdentChar(curChar)){
                identifier:
                //Continue if alphanumeric match doesnt end at keyword or no keyword exists and return identifier
                while(isIdentChar(curChar)) {
                    store(curChar);
                    getNext();
                }
                return tokIdent;
            }
            //Syntax error otherwise
            return tokUnexpected;
    } 
    assert(0 && "Lexer missing case");
}

const char_t * stringifyToken(Token tok){
    switch(tok){
        case tokEof:
            return "end of file";
        case tokReturn:
            return "keyword \"return\"";
        case tokInt:
            return "keyword \"int\"";
        case tokLong:
            return "keyword \"long\"";
        case tokFloat:
            return "keyword \"float\""; 
        case tokDouble:
            return "keyword \"double\"";
        case tokIdent:
            return "identifier";
        case tokNumDouble:
            return "floating-point number";
        case tokNumInt:
            return "integer";
        case tokString:
            return "string";
        case tokPlus:
            return "'+'";
        case tokMinus:
            return "'-'";
        case tokDiv:
            return "'/'";
        case tokMulti:
            return "'*'";
        case tokComma:
            return "','";
        case tokLParen:
            return "'('";
        case tokRParen:
            return "')'";
        case tokSemicolon:
            return "';'";
        case tokLBrace:
            return "'{'";
        case tokRBrace:
            return "'}'";
        default:
            assert(0 && "Unhandled token");
    }     
}