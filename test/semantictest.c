#include "semantics/semantics.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "ast/ast.h"
#include "ast/type.h"
#include "lexer/lexer.h"
#include "utils.h"
#include <string.h>

#include "test/utils/assert.h"
#include "test/utils/io.c"

// Specifically tests the AST transformations of the semantics module. 
// Error messages and symtable insertions are tested in the correctness integration tests

static void testVerifyLiteral(){
    initSemantics();

    ExprInt* integer = newExprInt(1, 2, 3);
    assertEqNum(verifyExprInt(integer)->base.type, typInt32);
    assertEqNum(verifyExprUnsignedInt(integer)->base.type, typUInt32);
    disposeAst(integer);

    ExprLong* longint = newExprLong(1, 2, 3);
    assertEqNum(verifyExprLong(longint)->base.type, typInt64);
    assertEqNum(verifyExprUnsignedLong(longint)->base.type, typUInt64);
    disposeAst(longint);

    ExprDouble* dbl = newExprDouble(1, 2, 5.5);
    assertEqNum(verifyExprDouble(dbl)->base.type, typFloat64);
    disposeAst(dbl);

    ExprFloat* flt = newExprFloat(1, 2, 5.5);
    assertEqNum(verifyExprFloat(flt)->base.type, typFloat32);
    disposeAst(flt);
}

static void testVerifyBinop(){
    static const Token arithOp[] = {tokPlus, tokMinus, tokDiv, tokMulti};
    #define arithCount 4
    static const Token relOp[] = {tokEquals, tokNotEquals, tokGreater, tokLess, tokGreaterEquals, tokLessEquals};
    #define relCount 6
    static const Token assignOp[] = {tokAssign, tokPlusAssign, tokMinusAssign, tokDivAssign, tokMultiAssign};
    #define assignCount 5

    for (int i=0; i<arithCount; i++){
        ExprBinop* binop = newExprBinop(
            1, 2, arithOp[i],
            (ExprBase*)verifyExprInt(newExprInt(1, 2, 3)),
            (ExprBase*)verifyExprUnsignedLong(newExprLong(1, 2, 3))
        );
        assertEqNum(verifyExprBinop(binop)->base.type, typUInt64);
        assertEqNum(binop->left->type, typUInt64);
        assertEqNum(binop->right->type, typUInt64);
        disposeAst(binop);
    }

    for (int i=0; i<relCount; i++){
        ExprBinop* binop = newExprBinop(
            1, 2, relOp[i],
            (ExprBase*)verifyExprInt(newExprInt(1, 2, 3)),
            (ExprBase*)verifyExprUnsignedLong(newExprLong(1, 2, 3))
        );
        assertEqNum(verifyExprBinop(binop)->base.type, typInt32);
        assertEqNum(binop->left->type, typUInt64);
        assertEqNum(binop->right->type, typUInt64);
        disposeAst(binop);
    }

    for (int i=0; i<assignCount; i++){
        ExprBinop* binop = newExprBinop(
            1, 2, assignOp[i],
            //Verifying the identifier requires the symbol table, so I set its type manually instead
            (ExprBase*)newExprIdent(1, 2, strdup("var")),
            (ExprBase*)verifyExprUnsignedLong(newExprLong(1, 2, 3))
        );
        binop->left->type = typUInt32;
        assertEqNum(verifyExprBinop(binop)->base.type, typUInt32);
        // Arith promotions should happen when doing arith assignments
        if (assignOp[i] != tokAssign){
            assertEqNum(binop->left->type, typUInt64);
            assertEqNum(binop->right->type, typUInt64);
        }
        disposeAst(binop);
    }
}

static void testVerifyUnop(){
    static const Token unops[] = {tokInc, tokDec, tokMinus, tokNot};
    #define unopCount 4
    for (int i=0; i<unopCount; i++){
        ExprUnop* unop = newExprUnop(1, 2, unops[i], (ExprBase*)newExprIdent(1, 2, strdup("ddd")), 1);
        unop->operand->type = typUInt32;
        if (unops[i] == tokNot){
            assertEqNum(verifyExprUnop(unop)->base.type, typInt32);
        }
        else{
            assertEqNum(verifyExprUnop(unop)->base.type, typUInt32);
        }
        disposeAst(unop);    
    }
}

static void testVerifyIdent(){
    initSemantics();
    initSymbolTable();

    char_t* varName = strdup("var");
    char_t* identName = strdup("var");
    StmtVar* var = newStmtVarDef(1, 2, typUInt8, varName);
    insertVar(varName, var);
    ExprIdent* ident = newExprIdent(1, 2, identName);
    assertEqNum(verifyExprIdent(ident)->base.type, var->type);
    disposeAst(var);
    disposeAst(ident);

    disposeSymbolTable();
}

static void testVerifyCall(){
    initSemantics();
    initSymbolTable();

    char_t* callName = strdup("func");
    char_t* funcName = strdup("func");
    Function* func = newFunction(1, 2, typFloat32, funcName);
    insertFunc(funcName, func);
    ExprCall* call = newExprCall(1, 2, callName);
    assertEqNum(verifyExprCall(call)->base.type, func->type);
    disposeAst(call);
    disposeAst(func);

    disposeSymbolTable();
}

static void testVerifyBlock(){
    initSemantics();
    initSymbolTable();

    StmtBlock* blk = newStmtBlock(1, 2);
    size_t before = curScope;
    preverifyBlockStmt();
    assertNotEqNum(curScope, before);
    size_t after = curScope;
    verifyBlockStmt(blk);
    postVerifyBlockStmt(blk);
    assertEqNum(blk->scopeId , after);
    assertEqNum(curScope, before);

    disposeAst(blk);
    disposeSymbolTable();
}

static void testVerifyFunctionDecl(){
    initSemantics();
    initSymbolTable();

    char_t* funcName = strdup("func");
    Function* func = newFunction(1, 2, typUInt64, funcName);
    size_t scopeId = curScope;
    verifyFunctionSignature(func, 1);
    assertEqNum(scopeId, curScope);
    assertNotEqNum(findFunc(funcName), NULL);

    disposeAst(func);
    disposeSymbolTable();
}

static void testVerifyFunctionDef(){
    initSemantics();
    initSymbolTable();

    char_t* funcName = strdup("func");
    Function* func = newFunction(1, 2, typUInt64, funcName);
    size_t before = curScope;
    verifyFunctionSignature(func, 0);
    assertNotEqNum(before, curScope);
    size_t after = curScope;
    assertEqNum(func->scopeId, after);
    assertNotEqNum(findFunc(funcName), NULL);
    verifyFunctionBody();
    assertEqNum(curScope, before);

    disposeAst(func);
    disposeSymbolTable();
}

int main(int argc, char const *argv[])
{
    testVerifyLiteral();
    testVerifyBinop();
    testVerifyUnop();
    testVerifyIdent();
    testVerifyCall();
    testVerifyBlock(); 
    testVerifyFunctionDecl();
    testVerifyFunctionDef();
    return 0;
}