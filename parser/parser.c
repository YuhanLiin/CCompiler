#include "array.h"
#include "lexer/lexer.h"
#include "ast/ast.h"
#include "utils.h"
#include "ast/type.h"
#include "io/error.h"
#include "semantics/semantics.h"
#include "./private.h"
#include <stdlib.h>
#include <assert.h>

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

//Checks for semicolon at end of statement, throwing error if it's not found
static void checkSemicolon(){
    if (curTok == tokSemicolon){
        getTok(); //Consume semicolon
    }
    else{  
        syntaxError(stringifyToken(tokSemicolon));              
    }
}

Ast* parseStmtOrDef();

static Ast* parseBlock(){
    StmtBlock* block = newStmtBlock(lineNumberTokStart, linePosTokStart);
    getTok(); //Consume left brace
    while (curTok != tokRBrace){
        Ast* stmt = parseStmtOrDef();
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
}

Ast* parseStmt(){
    size_t stmtLineNum = lineNumberTokStart;
    size_t stmtLinePos = linePosTokStart;
    switch(curTok){
        case tokReturn: {
            StmtReturn* ret = newStmtReturn(stmtLineNum, stmtLinePos);
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
            StmtEmpty* empty = newStmtEmpty(stmtLineNum, stmtLinePos);
            getTok(); //Consume semicolon
            return (Ast*) empty;
        }
        case tokLBrace: {
            preverifyBlockStmt();
            Ast* blk = parseBlock();
            postVerifyBlockStmt();
            return blk;
        }
        case tokWhile: {
            getTok();
            ExprBase* cond = parseBracketedExpr();
            if (cond){
                Ast* stmt = parseStmt();
                if (stmt){
                    return (Ast*)newStmtWhile(stmtLineNum, stmtLinePos, cond, stmt);
                }
                disposeAst(stmt);
            }
            disposeAst(cond);
            return NULL;
        }

        //These are tokens that expressions can't start with, so they automatically trigger statement error
        case tokRBrace:
        case tokRParen:
        case tokUnexpected:
        case tokEof:
        case tokMulti:
        case tokDiv:
        case tokComma:
        case tokInt:
        case tokLong:
        case tokDouble:
        case tokFloat:
        case tokChar:
        case tokSigned:
        case tokUnsigned:
            syntaxError("statement");
            return NULL;
        //Checks for expression/declaration statements as last resort
        default: {
            ExprBase* expr = parseExpr();
            if (expr){
                checkSemicolon();
                return (Ast*) newStmtExpr(expr->ast.lineNumber, expr->ast.linePos, expr);
            }
            return NULL;
        }
    }      
}

// Parse statement or variable definitions. Used inside braces
Ast* parseStmtOrDef(){
    if (canParseType()){
        Type type = parseType();
        if (type == typNone){
            syntaxError("type name");
        }
        else if (curTok == tokIdent){
            StmtVar* def = newStmtVarDef(lineNumberTokStart, linePosTokStart, type, toCstring(&stringBuffer));
            getTok();  //Consume identifier
            if (curTok == tokSemicolon){
                getTok();
                // Definition without assignment
                return (Ast*)verifyStmtVar(def);
            }
            else if (curTok == tokAssign){
                getTok();
                def->rhs = parseExpr();
                if (def->rhs){
                    // Definition plus assignment of value
                    checkSemicolon();
                    return (Ast*)verifyStmtVar(def);
                }
            }
            else{
                syntaxError("= or ;");
            }
            disposeAst(def);
        }
        else{
            syntaxError(stringifyToken(tokIdent));
        }
        return NULL;
    }
    else{
        return parseStmt();
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
                else if (curTok == tokLBrace){
                    func->stmt = parseBlock();
                    verifyFunctionBody();
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