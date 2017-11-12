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
    tokUnsigned,//Keyword unsigned
    tokSigned,  //Keyword signed
    tokChar,    //Keyword char
    tokIdent,   //Identifier [a-zA-Z][a-zA-Z_0-9]*  stored in stringBuffer
    tokNumDouble,  //64-bit floating pt literal .[0-9]+ | [0-9]+.[0-9]* in floatVal
    tokNumFloat,   //32-bit floating pt literal .[0-9]+ | [0-9]+.[0-9]*(f|F)
    tokNumULong,    //64-bit int literal unsigned [0-9]+(ll|LL)U
    tokNumLong,     //64-bit int literal [0-9]+(LL|ll)
    tokNumUInt, //32-bit int literal unsigned [0-9](l|L)?U
    tokNumInt,  //32-bit int literal [0-9]+(l|L)?
    tokNumChar, //'.'
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
extern size_t lineNumberTokStart;
extern size_t linePosTokStart;
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