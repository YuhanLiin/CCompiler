#include "lexer/lexer.h"
#include "ast/ast.h"
#include "utils.h"
#include "io/error.h"
#include "semantics/semantics.h"

// PRIVATE
Token curTok;  //Lookahead token. Global lexer values correspond to this token
static char correct = 1;

char checkSyntax(){
    return correct;
}

Token getTok(){
    return curTok = lexToken();
}

void syntaxError(const char_t* expected){
    if (curTok == tokUnexpected){
        if (curChar == End){
            writeError(lineNumber, linePos, "expected %s before end of file.", expected);
        }
        else{
            writeError(lineNumber, linePos, "expected %s before '%c'.", expected, curChar);
        }
    }
    else{
        writeError(lineNumberTokStart, linePosTokStart, "expected %s before %s.", expected, stringifyToken(curTok));
    }
    correct = 0;
}

// PUBLIC
void initParser(){
    getTok();
    correct = 1;
    initSemantics();
}