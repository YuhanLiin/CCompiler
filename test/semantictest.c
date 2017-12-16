#include "semantics/semantics.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "ast/ast.h"
#include "ast/type.h"
#include "lexer/lexer.h"
#include "utils.h"

#include "test/utils/assert.c"
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

static void testVerifyOperators(){
    ExprUnop* unop = newExprUnop(1, 2, tokMinus, (ExprBase*)verifyExprInt(newExprInt(1, 2, 3)), 1);
    assertEqNum(verifyExprUnop(unop)->base.type, typInt32);
    disposeAst(unop);

    ExprBinop* binop = newExprBinop(
        1, 2, tokPlus,
        (ExprBase*)verifyExprInt(newExprInt(1, 2, 3)),
        (ExprBase*)verifyExprUnsignedLong(newExprLong(1, 2, 3))
    );
    assertEqNum(verifyExprBinop(binop)->base.type, typUInt64);
    disposeAst(binop);

    New(char_t, identName, 4)
    strcpy(identName, "var");
    ExprBinop* assign = newExprBinop(
        1, 2, tokAssign,
        //Verifying the identifier requires the symbol table, so I set its type manually instead
        (ExprBase*)newExprIdent(1, 2, identName),
        (ExprBase*)verifyExprUnsignedLong(newExprLong(1, 2, 3))
    );
    assign->left->type = typUInt64;
    assertEqNum(verifyExprBinop(assign)->base.type, typUInt64);
    disposeAst(assign);
}

static void testVerifyIdent(){
    initSemantics();
    initSymbolTable();

    New(char_t, varName, 4)
    New(char_t, identName, 4)
    strcpy(varName, "var");
    strcpy(identName, "var");

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

    New(char_t, funcName, 5)
    New(char_t, callName, 5)
    strcpy(funcName, "func");
    strcpy(callName, "func");

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

    New(char_t, funcName, 5)
    strcpy(funcName, "func");

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

    New(char_t, funcName, 5)
    strcpy(funcName, "func");

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
    testVerifyOperators();
    testVerifyIdent();
    testVerifyCall();
    testVerifyBlock(); 
    testVerifyFunctionDecl();
    testVerifyFunctionDef();
    return 0;
}