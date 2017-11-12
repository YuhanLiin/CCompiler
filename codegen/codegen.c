#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "ast/ast.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "codegen/address.h"
#include "codegen/codegen.h"

static size_t maxLabelNum = 0;
static const Register paramRegisters[] = {$rcx, $rdx, $r8, $r9};

static const void emitLabelStmt(const char_t *op, size_t label){
    emitAsm("\t%s .L%d\n", op, label);
}
static const void emitLabelDecl(size_t label){
    emitAsm(".L%d:\n", label);
}

// Never use this as a format string
static const char_t* registerStr(Register reg){
    switch (reg){
        case $rax:
            return "%rax";
        case $rbp:
            return "%rbp";
        case $rsp:
            return "%rsp";
        case $rcx:
            return "%rcx";
        case $rdx:
            return "%rdx";
        case $r8:
            return "%r8";
        case $r9:
            return "%r9";
        default:
            assert(0 && "Unsupported register type");
    }
}

static void emitAddr(Address addr){
    switch (addr.mode){
        case registerMode:
            emitAsm("%s", registerStr(addr.val.reg));
            break;
        case symbolMode:
            emitAsm(addr.val.symbol);
            break;
        case numberMode:
            emitAsm("$%lld", addr.val.num);
            break;
        case indirectMode:
            emitAsm("%lld(%s)", addr.val.indirect.offset, registerStr(addr.val.indirect.reg));
            break;
        default:
            assert(0 && "Unsupported addressing mode");
    }
}

static Address cmplExpr(ExprBase* ast);

// Args array ptr can be null, signifying a call with no args
static void cmplCall(char_t* name, Array(vptr) *args){
    if (args != NULL){
        int i;
        Address expraddr;
        // Push each argument onto the stack from right to left
        for (i=args->size-1; i>=4; i--){
            expraddr = cmplExpr((ExprBase*)args->elem[i]);
            emitAsm("\tpushq ");
            emitAddr(expraddr);
            emitAsm("\n");
        }
        // For the last 4 args, put them into registers instead
        emitAsm("\tsubq $32, %s\n", registerStr($rsp));
        for (i; i>=0; i--){
            expraddr = cmplExpr((ExprBase*)args->elem[i]);
            emitAsm("\tmovq ");
            emitAddr(expraddr);
            emitAsm(", %s\n", registerStr(paramRegisters[i]));
        }
    }
    else{
        emitAsm("\tsubq $32, %s\n", registerStr($rsp));
    }
    emitAsm("\tcall %s\n", name);
}

// Returns address of the result of the processed expression
static Address cmplExpr(ExprBase* expr){
    switch(expr->base.ast.label){
        case astExprInt:
            return numberAddress(((ExprInt*)expr)->num);
        case astExprCall:
            cmplCall(((ExprCall*)expr)->name, &((ExprCall*)expr)->args);
            return registerAddress($rax);
        case astExprIdent:
            return findAddress(((ExprIdent*)expr)->name);
        default:
            assert(0 && "Unsupported AST for expr");
    }
}

static void cmplStmt(Ast* ast, size_t retLabel){
    switch(ast->label){
        case astStmtReturn: {
            Address expaddr = cmplExpr((ExprBase*)((StmtReturn*)ast)->expr);
            emitAsm("\tmovq ");
            emitAddr(expaddr);
            emitAsm(", %s\n", registerStr($rax));
            emitLabelStmt("jmp", retLabel);
            break;
        }
        case astStmtEmpty:
            break;
        case astStmtBlock: {
            StmtBlock* blk = (StmtBlock*)ast;
            curScope = blk->scopeId;
            for (size_t i=0; i<blk->stmts.size; i++){
                cmplStmt(blk->stmts.elem[i], retLabel);
            }
            toPrevScope();
            break;
        }
        default:
            assert(0 && "Unsupported AST for stmt");
    }
}

static void cmplParams(Array(vptr) *params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        Address location = indirectAddress(16 + i*8, $rbp);
        // Right now dumps all param registers into shadow space. Safe but inefficient
        if (i < 4){
            emitAsm("\tmovq %s, ", registerStr(paramRegisters[i]));
            emitAddr(location);
            emitAsm("\n");
        }
        insertAddress(param->name, location);
    }
}

static void cmplGlobal(Ast* ast){
    switch(ast->label){
        case astFunction:{
            Function* func = (Function*)ast;
            //Skip function declarations
            if (isFuncDecl(func)){
                return;
            }
            emitAsm(".globl %s\n", func->name);
            emitAsm("%s:\n", func->name);
            emitAsm("\tpushq %s\n", registerStr($rbp));
            emitAsm("\tmovq %s, %s\n", registerStr($rsp), registerStr($rbp));

            // Set global scope variable so that symbol table can be used
            curScope = func->scopeId;
            // Create a new label for the return location
            size_t retLabel = maxLabelNum++;
            cmplParams(&func->params);
            if (!strcmp(func->name, "main")){
                cmplCall("__main", NULL);
            }
            cmplStmt((Ast*)func->stmt, retLabel);
            // Return code
            emitLabelDecl(retLabel);
            emitAsm("\tleave\n");
            emitAsm("\tret\n");
            // Reset scope back to global
            curScope = GLOBAL_SCOPE;
            break;
        }
        default:
            assert(0 && "Invalid AST for top level");
    }
}

void cmplTopLevel(TopLevel* top){
    initAddrTable();
    emitAsm(".text\n");
    for (size_t i=0; i<top->globals.size; i++){
        cmplGlobal(top->globals.elem[i]);
    }
    disposeAddrTable();
}