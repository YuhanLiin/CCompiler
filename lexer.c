//#include <stdio.h>
#include "array.h"
#include "lexer.h"
#include "utils.h"
#include <assert.h>

char_t curChar; //Updated for every character consumed
Array(char_t) stringBuffer; //Store identifier and strings
double floatVal;   //Store number tokens
long long intVal;

void initLexer(){ //Can fail due to malloc
    if (!initArr(char_t)(&stringBuffer, 5, NULL, NULL)) exit(1); //Allocate empty string
}

void disposeLexer(){
    disposeArr(char_t)(&stringBuffer);
}

//Consumes char and updates curChar. Called by tokenizer
static char_t getNext(){
    curChar = consumeNext();
    return curChar;
}
//Store in string
static char_t store(char_t c){
    pushArr(char_t)(&stringBuffer, c);
    return c;
}
//Get next char if it matches c. REturns whether match occurs
static char getNextIf(char_t c){
    if (peekNext() == c){
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
    return isAlpha(c) || isDigit(c) || c == '_';
}

//Recognizes decimal sequence and adds it to floatVal. Returns whether decimals were recognized
static char lexDecimals(){
    if (isDigit(peekNext())){
        double d = 0.1;
        do{
            floatVal += d*(getNext()-'0');
            d /= 10;
        } while(isDigit(peekNext()));
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
    if (isIdentChar(peekNext())){
        return 0;
    }
    return 1;
}

//Gets the next token
Token lexToken(){
    //Reset fields
    clearArr(char_t)(&stringBuffer);
    floatVal = 0;
    intVal = 0;
    curChar = 0;

    begin:
    //Skip over newlines, whitespace
    while(isSpace(peekNext()) || isEol(peekNext())){
        getNext();
    }   
    switch (peekNext()) {
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
            while (peekNext() != '"'){
                //If file ends in middle of a string return token for unexpected char
                if (peekNext() == End){
                    getNext();
                    return tokUnexpected;
                }
                store(getNext());
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
            if (peekNext() == '/'){
                getNext();
                //Comment ends when end of line of end of file is seen
                while (!isEol(peekNext()) || peekNext() != End){
                    getNext();
                }
                goto begin; //No token to return, so start lexing again
            }
            //Multi line comment
            else if (peekNext() == '*'){
                getNext();  //Consume *
                do {
                    if (peekNext() == End){
                        return tokUnexpected;
                    }
                    getNext();
                  //If the last consumed comment char is * and the one after it is /, then comment is over
                } while (curChar != '*' || peekNext() != '/');
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
            if (isDigit(peekNext())){
                do{ //Convert first set of digits as integer
                    intVal *= 10;
                    intVal += getNext() - '0';
                }while(isDigit(peekNext()));
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
            else if (isIdentChar(peekNext())){
                identifier:
                //Continue if alphanumeric match doesnt end at keyword or no keyword exists and return identifier
                do {
                    store(getNext());
                } while(isIdentChar(peekNext()));
                return tokIdent;
            }
            //Syntax error otherwise
            return tokUnexpected;
    } 
    assert(0 && "Lexer missing case");
}

