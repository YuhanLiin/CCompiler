#pragma once
#include "array.h"
#include "utils.h"

//All possible tokens
typedef enum {
    tokUnexpected,    //Unexpected character found. Consult curChar
    tokEof,     //End of the file is matched   
    tokReturn,  //Keyword return
    tokInt,     //keyword int
    tokLong,    //keyword long
    tokFloat,   //keyword float
    tokDouble,  //keyword double
    tokIdent,   //Identifier [a-zA-Z][a-zA-Z_0-9]*  stored in stringBuffer
    tokNumDouble,  //64-bit floating pt literal .[0-9]+ | [0-9]+.[0-9]* in floatVal
    tokNumInt,     //64-bit int literal [0-9]+
    tokString,  //String literal "[anychar]" stored in stringBuffer
    tokPlus,    //Operator +
    tokMinus,   //Operator -
    tokDiv,     //Operator /    
    tokMulti,   //Operator *
    tokComma,   // , token 
    tokLParen,      // ( token
    tokRParen,      // ) token
    tokSemicolon,
    tokLBrace,
    tokRBrace,
} Token;

#define End 0 //Eof character
extern size_t lineNumber;
extern size_t linePos;
extern size_t lineTokenEndNumber;
extern size_t lineTokenEndPos;
extern Array(char_t) stringBuffer; //Store identifier and strings
extern double floatVal; 
extern long long intVal;
extern char_t curChar;

void initLexer();   //Allocates stringBuffer. Can exit due to malloc
void disposeLexer();//Dispose stringBuffer
//Public functions for peeking and consuming from character stream. Defined elsewhere
char_t consumeNext(); //Called by tokenizer
//Gets the next token
Token lexToken();
const char_t * stringifyToken(Token tok);