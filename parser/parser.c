#include "array.h"
#include "lexer/lexer.h"
#include "ast/ast.h"
#include "utils.h"
#include "./parser.h"
#include "ast/type.h"
#include "io/error.h"
#include "semantics/semantics.h"
#include "semantics/symtable.h"
#include "scope/scope.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>

static Token curTok;  //Lookahead token. Global lexer values correspond to this token
static char correct = 1;

static Token getTok(){
    return curTok = lexToken();
}

char checkSyntax(){
    return correct;
}

static void syntaxError(const char_t* expected){
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

void initParser(){
    getTok();
    correct = 1;
    initSemantics();
}

//IMPORTANT: Extra unprocessed lookahead will always be present at start and end of every parser function call
//Return 0 for syntax error and stop processing tokens

static char canParseType(){
    switch(curTok){
        case tokChar:
        case tokVoid:
        case tokShort:
        case tokInt:
        case tokLong:
        case tokFloat:
        case tokDouble:
        case tokSigned:
        case tokUnsigned:
            return 1;
        default:
            return 0;
    }
}

//Parses and returns signed versions of all integers types.
static Type parseIntegerType(){
    switch (curTok){
        case tokChar:
            getTok();
            return typInt8;
        case tokShort:
            getTok();
            if (curTok == tokInt){
                getTok();
            }
            return typInt16;
        case tokInt:
            getTok();
            return typInt32;
        case tokLong:
            getTok();
            if (curTok == tokLong){
                getTok();
                if (curTok == tokInt){
                    getTok();
                }
                return typInt64;
            }
            else if (curTok == tokInt){
                getTok();
            }
            return typInt32;
        default:
            return typNone;
    }
}

//Attempt to parse and return a built in type
static Type parseType(){
    switch(curTok){
        case tokDouble:
            getTok();
            return typFloat64;
        case tokFloat:
            getTok();
            return typFloat32;
        case tokVoid:
            getTok();
            return typVoid;
    }
    // Signed/Unsigned types start here
    if (curTok == tokUnsigned){
        getTok();
        switch(parseIntegerType()){
            case typInt8: return typUInt8;
            case typInt16: return typUInt16;
            case typInt32: return typUInt32;
            case typInt64: return typUInt64;
            default: return typNone;
        }
    }
    else if (curTok == tokSigned){
        getTok();
    }
    return parseIntegerType();
}

static ExprBase* parseExpr();

//args := expr [, expr]*   Assumes args array is already initialized and empty. Return 0 for syntax error
static char parseArgs(Array(vptr) *args){
    //While comma exists consume it and keep parsing expressions
    do {
        ExprBase* arg = parseExpr();
        if (arg == NULL) return 0; //If expression cant be parsed then syntax error
        if (!arrPush(vptr)(args, arg)) exit(1); //Push arg and check for malloc failures
    }while (curTok == tokComma && (getTok() || 1));   
    return 1;
}

//primeExpr := Number | String | ( expr ) | Ident ( args? )?
static ExprBase* parsePrimaryExpr(){
    switch (curTok){
        case tokNumInt: {
            ExprInt* expr = verifyExprInt(newExprInt(lineNumberTokStart, linePosTokStart, intVal));
            getTok();
            return (ExprBase*)expr;
        }
        case tokNumUInt: {
            ExprInt* expr = verifyExprUnsignedInt(newExprInt(lineNumberTokStart, linePosTokStart, intVal));
            getTok();
            return (ExprBase*)expr;
        }
        case tokNumLong: {
            ExprLong* expr = verifyExprLong(newExprLong(lineNumberTokStart, linePosTokStart, intVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumULong: {
            ExprLong* expr = verifyExprUnsignedLong(newExprLong(lineNumberTokStart, linePosTokStart, intVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumFloat: {
            ExprFloat* expr = verifyExprFloat(newExprFloat(lineNumberTokStart, linePosTokStart, floatVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokNumDouble: {
            ExprDouble* expr = verifyExprDouble(newExprDouble(lineNumberTokStart, linePosTokStart, floatVal));
            getTok(); //Consume number
            return (ExprBase*)expr;
        }
        case tokString: {
            ExprStr* expr = newExprStr(lineNumberTokStart, linePosTokStart, toCstring(&stringBuffer));
            getTok(); //Consume string
            return (ExprBase*)expr;
        }
        case tokLParen: {
            getTok(); //Consume left paren
            ExprBase* expr = parseExpr();
            if (expr){
                //Look for right paren. If not found just pretend it's there and write to error
                if (curTok == tokRParen){
                    getTok();
                }
                else {
                    syntaxError(stringifyToken(tokRParen));
                }
                return expr;
                disposeAst(expr);
            }
            return NULL;
        }
        case tokIdent: {
            char_t* name = toCstring(&stringBuffer);
            size_t nameline = lineNumberTokStart;
            size_t namepos = linePosTokStart;
            getTok(); //consume identifier
            //If bracket follows then its a function call
            if (curTok == tokLParen){  
                getTok(); //Consume left paren
                ExprCall* call = newExprCall(nameline, namepos, name);
                //If following brackets are not empty, attempt to parse 1 or more args.
                if (curTok == tokRParen){
                    getTok(); //Consume rb
                    return (ExprBase*)verifyExprCall(call);
                } 
                else{
                    //Attempt to parse arguments. It will throw if unsuccessful so no need to handle it here
                    if (parseArgs(&call->args)){
                        if (curTok == tokRParen){
                            getTok(); //Consume rb
                        }
                        else{
                            syntaxError(stringifyToken(tokRParen));
                        }
                        return (ExprBase*)verifyExprCall(call);
                    }
                }
                arrDispose(vptr)(&call->args);
                free(call);
            }
            //Otherwise its a normal identifier
            else {
                return (ExprBase*)verifyExprIdent(newExprIdent(nameline, namepos, name));
            }
            free(name);
            return NULL;
        }
        default:
            syntaxError("expression");
            return NULL;
    }
}

static ExprBase* parseLeftUnopExpr(){
    switch(curTok){
        //All single-token unary operators go here
        case tokMinus: {
            size_t opLine = lineNumber;
            size_t opPos = linePos;
            Token op = curTok;
            getTok();
            ExprBase* operand = parseLeftUnopExpr();
            if (operand){
                return (ExprBase*)verifyExprLeftUnop(newExprLeftUnop(lineNumber, linePos, op, operand));
            }
            return NULL;
        }
        //Plus operator means nothing, so just skip it
        case tokPlus:
            getTok(); 
            return parseLeftUnopExpr();
        //TODO this should be something like parseRightUnaryExpr
        default: return parsePrimaryExpr();
    }
}

//Return precedence of all operator tokens, 0 if token isnt operator
static int operatorPrec(Token op){
    switch (op){
        case tokPlus: return 1;
        case tokMinus: return 1;
        case tokMulti: return 2;
        case tokDiv: return 2;
        default: return 0;
    }
}

//Precedence climbing algorithm for binops. Constructs lhs from subsequent terms. Will not free lhs. Returns updated lhs or error
//exprBinop := [ [+-/*] primeExpr]*
static ExprBase* parseBinopExpr(ExprBase* lhs, int minPrec){
    int prec;
    ExprBase* rhs = NULL;
    Token op;
    //Will first consume any higher/equal precedence binop. Afterwards, due to inner loop, this outer loop will only consume equal precedence binops
    while (operatorPrec(op = curTok) >= minPrec){
        size_t opLine = lineNumber;
        size_t opPos = linePos;
        getTok(); //Consume binop
        //Attempt to parse 1st atom of rhs expression
        if ((rhs = parseLeftUnopExpr()) == NULL) return rhs;
        //While next binop is of higher precedence, accumulate expression into rhs
        while ((prec = operatorPrec(curTok)) > minPrec){
            ExprBase* newRhs = parseBinopExpr(rhs, prec);
            //Attempt to parse subsequent atoms at a precedece equal to current binop
            if (newRhs == NULL){
                disposeAst(rhs); //Responsible for delete rhs, which was created in this function
                return newRhs;
            }
            rhs = newRhs;
        }
        //After rhs has been fully built, merge it with lhs and then continue
        lhs = (ExprBase*)verifyExprBinop(newExprBinop(opLine, opPos, op, lhs, rhs));
    }
    return lhs;
}

//expr := primeExpr exprBinop
static ExprBase* parseExpr(){
    ExprBase* lhs = parseLeftUnopExpr(); //Parse the 1st primary expression
    if (lhs == NULL) return lhs; 
    ExprBase* expr = parseBinopExpr(lhs, 1); //Parse all following binops
    if (expr == NULL){
        disposeAst(lhs);
    }
    return expr;
}

//Checks for semicolon at end of statement, throwing error if it's not found
static void checkSemicolon(){
    if (curTok == tokSemicolon){
        getTok(); //Consume semicolon
    }
    else{  
        syntaxError(stringifyToken(tokSemicolon));              
    }
}

Ast* parseStmt(){
    switch(curTok){
        case tokReturn: {
            StmtReturn* ret = newStmtReturn(lineNumberTokStart, linePosTokStart);
            getTok(); //Consume return
            if (curTok == tokSemicolon){
                getTok();
            }
            else{
                ExprBase* expr = parseExpr();
                if (expr){
                    ret->expr = expr;
                    checkSemicolon();
                }
                else{
                    disposeAst(ret);
                    return NULL;
                }
            }
            return (Ast*)verifyStmtReturn(ret);
        }
        case tokSemicolon: {
            StmtEmpty* empty = newStmtEmpty(lineNumberTokStart, linePosTokStart);
            getTok(); //Consume semicolon
            return (Ast*) empty;
        }
        case tokLBrace: {
            StmtBlock* block = newStmtBlock(lineNumberTokStart, linePosTokStart);
            getTok(); //Consume left brace
            preverifyBlockStmt();
            while (curTok != tokRBrace){
                Ast* stmt = parseStmt();
                if (stmt){
                    if (!arrPush(vptr)(&block->stmts, stmt)){
                        exit(1);
                    }
                }
                else{
                    disposeAst(block);
                    return NULL;
                }
            }
            getTok();
            return (Ast*)verifyBlockStmt(block);
            //TODO handle other statements
        }

        //These are tokens that expressions can't start with, so they automatically trigger statement error
        case tokRBrace:
        case tokRParen:
        case tokUnexpected:
        case tokEof:
        case tokMulti:
        case tokDiv:
        case tokComma:
            syntaxError("statement");
            return NULL;
        //Checks for expression/declaration statements as last resort
        default: {
            if (canParseType()){
                Type type = parseType();
                if (type == typNone){
                    syntaxError("type name");
                    return NULL;
                }
                if (curTok == tokIdent){
                    StmtVar* def = newStmtVarDef(lineNumberTokStart, linePosTokStart, type, toCstring(&stringBuffer));
                    getTok();  //Consume identifier
                    checkSemicolon();
                    //TODO write verifyer for this or throw it somewhere else
                    return (Ast*) def;
                }
                else{
                    syntaxError(stringifyToken(tokIdent));
                }
            }
            else {
                ExprBase* expr = parseExpr();
                if (expr){
                    checkSemicolon();
                    return (Ast*) newStmtExpr(expr->ast.lineNumber, expr->ast.linePos, expr);
                }
            }
            return NULL;
        }
    }      
}

static char parseParams(Array(vptr) *params){
    //While comma exists, consume it (the getTok() call) and keep parsing identifiers
    do {
        size_t paramline = lineNumber;
        size_t parampos = linePos;
        Type type = parseType();
        //Each param must consist of a type and a name
        if (type == typNone){ 
            syntaxError("type name");
            return 0;
        }
        StmtVar* param = newStmtVarDef(paramline, parampos, type, NULL);
        if (curTok == tokIdent){
            param->name = toCstring(&stringBuffer);
            getTok();  //Consume name
        }
        if (!arrPush(vptr)(params, param)){
            exit(1);  //Push param and check for malloc failures
        }
    } while (curTok == tokComma && (getTok() || 1));
    return 1;
}

//function := type ident ( params? ) stmt
static Ast* parseFunction(){
    Type type = parseType();
    if (type == typNone){
        syntaxError("type name");
        return NULL;
    }
    if (curTok == tokIdent){
        char_t* name = toCstring(&stringBuffer);
        size_t nameline = lineNumberTokStart;
        size_t namepos = linePosTokStart;
        getTok(); //Consume identifier
        if (curTok == tokLParen){
            getTok(); //Consume left paren
            Function* func = newFunction(nameline, namepos, type, name);
            if (curTok == tokRParen){
                getTok(); //Consume right paren
                goto finishedParsingParams;
            }
            // Parse parameters. This will report errors so no need to do it here
            else if (parseParams(&func->params)){
                //Right paren error recovery
                if (curTok == tokRParen){
                    getTok(); //Consume right paren
                }
                else{
                    syntaxError(stringifyToken(tokRParen));
                }
                finishedParsingParams:;
                char isDecl = curTok == tokSemicolon;
                verifyFunctionSignature(func, isDecl);
                //Parse either ; for declaration or a statement for definition
                if (isDecl){
                    getTok();
                    return (Ast*)func;
                }
                else if (func->stmt = parseStmt()){
                    verifyFunctionBody();
                    return (Ast*)func;
                }
                //parseStmt reports errors, so no need for it here
            }
            arrDispose(vptr)(&func->params);
            free(func);
        }
        //TODO declarations as well
        free(name);
    }
    else{
        syntaxError(stringifyToken(tokIdent));
    }
    return NULL;
}

static Ast* parseGlobal(){
    if (canParseType()){
        return parseFunction();
    }
    else{
        syntaxError("type name");
        //TODO handle non-functions
        return NULL;
    }
}

TopLevel* parseTopLevel(){
    TopLevel* toplevel = newTopLevel();
    while(curTok != tokEof){
        Ast* ast = parseGlobal();
        if (ast){
            if (!arrPush(vptr)(&toplevel->globals, ast)){
                exit(1);
            }
        }
        else{
            disposeAst(toplevel);
            return NULL;
        }
    }
    return toplevel;
}