#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/ast.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "codegen/address.h"
#include "codegen/codegen.h"

//IMPORTANT Right now everything is in 64-bit mode, including constants. 
//This means upper bits will always be extended out so no danger for now. Type conversion will need to be handled when this ends

// # of labels used so far
static size_t maxLabelNum = 0;
// Distance of rsp from rbp
static int64_t frameOffset = 0;
// Registers used as params in the MS x64 calling convention
static const Register paramRegisters[] = {$rcx, $rdx, $r8, $r9};

static void emitLabelStmt(const char_t *op, size_t label){
    emitAsm("\t%s .L%d\n", op, label);
}
static void emitLabelDecl(size_t label){
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
        case $r10:
            return "%r10";
        case $r11:
            return "%r11";
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
            emitAsm("$%llu", addr.val.num);
            break;
        case indirectMode:
            emitAsm("%lld(%s)", addr.val.indirect.offset, registerStr(addr.val.indirect.reg));
            break;
        default:
            assert(0 && "Unsupported addressing mode");
    }
}

static void emitIns0(const char_t* opcode){
    emitAsm("\t%s\n", opcode);
}
static void emitIns1(const char_t* opcode, Address a){
    emitAsm("\t%s ", opcode);
    emitAddr(a);
    emitAsm("\n");
}
static void emitIns2(const char_t* opcode, Address a, Address b){
    emitAsm("\t%s ", opcode);
    emitAddr(a);
    emitAsm(", ");
    emitAddr(b);
    emitAsm("\n");
}

static void emitPush(Address a){
    emitIns1("pushq", a);
    frameOffset -= 8;
}
// rsp should only be offsetted by a compiled-time value
static void emitSubRsp(int64_t num){
    if (num != 0){
        emitIns2("subq", numberAddress(num), registerAddress($rsp));
        frameOffset -= num;
    }
}
static void emitAddRsp(int64_t num){
    if (num != 0){
        emitIns2("addq", numberAddress(num), registerAddress($rsp));
        frameOffset += num;
    }
}

static Address cmplExpr(ExprBase* ast);

// Args array ptr can be null, signifying a call with no args
static void cmplCall(char_t* name, Array(vptr) *args){
    if (args != NULL){
        Address argAddrs[args->size];
        // First compute all args and push onto stack if its a register
        for (size_t i=0; i<args->size; i++){
            argAddrs[i] = cmplExpr((ExprBase*)args->elem[i]);
            if (argAddrs[i].mode == registerMode){
                emitPush(argAddrs[i]);
                argAddrs[i] = indirectAddress(frameOffset, $rbp);
            }
        }
        int j;
        // Push each argument onto the stack from right to left
        for (j=args->size-1; j>=4; j--){
            emitPush(argAddrs[j]);
        }
        // For the last 4 args, put them into registers instead
        for (j; j>=0; j--){
            emitIns2("movq", argAddrs[j], registerAddress(paramRegisters[j]));
        }
    }
    emitSubRsp(32);
    emitIns1("call", symbolAddress(name));
}

static Address cmplUnop(ExprUnop* unop){
    Address addr = cmplExpr(unop->operand);
    // If it's a leftside increment/decrement, then just update and return the operand without a temp value
    if (unop->leftside){
        switch(unop->op){
            case tokInc:
                emitIns1("incq", addr);
                return addr;
            case tokDec:
                emitIns1("decq", addr);
                return addr;
        }
    }
    emitPush(addr);
    Address temp = indirectAddress(frameOffset, $rbp);
    switch(unop->op){
        case tokMinus:
            emitIns1("negq", temp);
            break;
        // Rightside increment and decrement
        case tokInc:
            emitIns1("incq", addr);
            break;
        case tokDec:
            emitIns1("decq", addr);
            break;
        default:
            assert(0 && "Not a token unary operator");
    }
    return temp;
}

// r10 will be used as intermediate for all binop operations
const Register binopIntermediate = $r10;

//Multiply left by right and store in rax. Promotes result to type
static void emitMulti(Address left, Address right, Type type){
    assert(left.mode == indirectMode && "Left operand must be on stack");
    //Put right operand on rax if its not already there
    if (right.mode != registerMode || right.val.reg != $rax){
        emitIns2("movq", right, registerAddress($rax));
        right = registerAddress($rax);
    }
    if (isSignedType(type)){
        emitIns1("imulq", left);
    }
    else{
        emitIns1("mulq", left);
    }
}
static void emitDiv(Address left, Address right, Type type){
    assert(left.mode == indirectMode && "Left operand must be on stack");
    //Can't run div instruction on a number. Also must have left operand on rax, so right operand can't be on rax
    if (right.mode == numberMode || (right.mode == registerMode && right.val.reg == $rax)){
        emitIns2("movq", right, registerAddress(binopIntermediate));
        right = registerAddress(binopIntermediate);
    }
    emitIns2("movq", left, registerAddress($rax));
    if (isSignedType(type)){
        emitIns0("cqto");
        emitIns1("idivq", right);
    }
    else{
        emitIns2("movq", numberAddress(0), registerAddress($rdx));
        emitIns1("divq", right);
    }
}
// Perform specified arithemtic operation on operands and store value in left operand
static void emitBinop(const char_t* op, Address left, Address right, Type type){
    // Can't have both operands on stack, so move second one to intermediate register. 
    if (right.mode == indirectMode){
        emitIns2("movq", right, registerAddress(binopIntermediate));
        right = registerAddress(binopIntermediate);
    }
    emitIns2(op, right, left);
}

// Move left operand onto stack and destructively operate on it
static Address cmplBinop(ExprBinop* binop){    
    Address left = cmplExpr(binop->left);
    // Move left operand onto the stack unless we're assigning to it directly
    if (!isAssignmentOp(binop->op)){
        emitPush(left);
        left = indirectAddress(frameOffset, $rbp);
    }
    Address right = cmplExpr(binop->right);
    
    // Left operand will always be the destination operand that is mutated
    switch(binop->op){
        case tokAssign:
            emitBinop("movq", left, right, binop->base.type);
            return left;
        case tokPlusAssign:
        case tokPlus:
            emitBinop("addq", left, right, binop->base.type);
            return left;
        case tokMinusAssign:
        case tokMinus:
            emitBinop("subq", left, right, binop->base.type);
            return left;
        case tokMulti:
            emitMulti(left, right, binop->base.type);
            return registerAddress($rax);
        case tokMultiAssign:
            emitMulti(left, right, arithTypePromotion(binop->left->type, binop->right->type));
            emitIns2("movq", registerAddress($rax), left);
            return left;
        case tokDiv:
            emitDiv(left, right, binop->base.type);
            return registerAddress($rax);
        case tokDivAssign:
            emitDiv(left, right, arithTypePromotion(binop->left->type, binop->right->type));
            emitIns2("movq", registerAddress($rax), left);
            return left;
        default:
            assert(0 && "Unhandled binop.");
    }
}

// Returns address of the result of the processed expression
static Address cmplExpr(ExprBase* expr){
    assert(
        (expr->ast.label == astExprCall || expr->type != typVoid) && 
        "Other than calls, void expressions can't exist."
    );
    assert(expr->type != typNone && "None expressions can't exist at all.");
    switch(expr->ast.label){
        case astExprInt:
            return numberAddress(((ExprInt*)expr)->num);
        case astExprLong:
            return numberAddress(((ExprLong*)expr)->num);
        case astExprCall:
            cmplCall(((ExprCall*)expr)->name, &((ExprCall*)expr)->args);
            return registerAddress($rax);
        case astExprIdent:
            return findAddress(((ExprIdent*)expr)->name);
        case astExprBinop: {
            return cmplBinop((ExprBinop*)expr);
        }
        case astExprUnop: 
            return cmplUnop((ExprUnop*)expr);
        default:
            assert(0 && "Unsupported AST for expr");
    }
}

static void cmplStmt(Ast* ast, size_t retLabel){
    switch(ast->label){
        case astStmtEmpty:
            break;
        case astStmtExpr:
            cmplExpr(((StmtExpr*)ast)->expr);
            break;
        case astStmtReturn: {
            if (hasRetExpr((StmtReturn*)ast)){
                Address expaddr = cmplExpr((ExprBase*)((StmtReturn*)ast)->expr);
                emitIns2("movq", expaddr, registerAddress($rax));
            }
            emitLabelStmt("jmp", retLabel);
            break;
        }
        case astStmtBlock: {
            StmtBlock* blk = (StmtBlock*)ast;
            curScope = blk->scopeId;
            for (size_t i=0; i<blk->stmts.size; i++){
                cmplStmt(blk->stmts.elem[i], retLabel);
            }
            toPrevScope();
            break;
        }
        case astStmtDef: {
            StmtVar* def = (StmtVar*)ast;
            if (def->rhs){
                emitPush(cmplExpr(def->rhs));
            }
            else{
                emitSubRsp(8);
            }
            insertAddress(def->name, indirectAddress(frameOffset, $rbp));
            break;
        }
        case astStmtWhile: {
            //TODO Get rid of the rsp adds when gotos are implemented
            StmtWhileLoop* loop = (StmtWhileLoop*)ast;
            size_t loopStart = maxLabelNum++;
            int64_t startOffset = frameOffset;

            //Evaluate condition after start label
            emitLabelDecl(loopStart);
            Address cond = cmplExpr(loop->condition);
            if (cond.mode == numberMode){
                //If cond is 0 then skip everything, otherwise just loop back to start label infinitely
                if (cond.val.num != 0){
                    cmplStmt(loop->stmt, retLabel);
                    emitAddRsp(startOffset - frameOffset);
                    emitLabelStmt("jmp", loopStart);
                }
            }
            else{
                //If condition evaluates to 0 then exit loop via jump
                size_t loopEnd = maxLabelNum++;
                emitIns2("cmpq", numberAddress(0), cond);
                emitLabelStmt("je", loopEnd);
                //Otherwise evaluate inner statement then go back up
                cmplStmt(loop->stmt, retLabel);
                emitAddRsp(startOffset - frameOffset);
                emitLabelStmt("jmp", loopStart);
                emitLabelDecl(loopEnd);
            }
            break;
        }
        case astStmtDoWhile: {
            StmtWhileLoop* loop = (StmtWhileLoop*)ast;
            size_t loopStart = maxLabelNum++;
            int64_t startOffset = frameOffset;

            //Evaluate statement then condition
            emitLabelDecl(loopStart);
            cmplStmt(loop->stmt, retLabel);
            Address cond = cmplExpr(loop->condition);
            emitAddRsp(startOffset - frameOffset);
            //Special treatment for const conditions
            if (cond.mode == numberMode){
                if (cond.val.num != 0){
                    emitLabelStmt("jmp", loopStart);
                }
            }
            //If cond is not 0 jump back up and loop again
            else{
                emitIns2("cmpq", numberAddress(0), cond);
                emitLabelStmt("jne", loopStart);
            }
            break;
        }
        default:
            assert(0 && "Unsupported AST for stmt");
    }
}

static void cmplParams(Array(vptr) *params){
    for (size_t i=0; i<params->size; i++){
        StmtVar* param = params->elem[i];
        assert(param->type != typNone && param->type != typVoid && "Can't have void/none params");
        Address location = indirectAddress(16 + i*8, $rbp);
        // Right now dumps all param registers into shadow space. Safe but inefficient
        if (i < 4){
            emitIns2("movq", registerAddress(paramRegisters[i]), location);
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
            emitIns1("pushq", registerAddress($rbp));
            emitIns2("movq", registerAddress($rsp), registerAddress($rbp));

            // Set global scope variable so that symbol table can be used
            curScope = func->scopeId;
            // Create a new label for the return location
            size_t retLabel = maxLabelNum++;
            // $rbp = $rsp, so set the offset to 0
            frameOffset = 0;
            cmplParams(&func->params);
            if (!strcmp(func->name, "main")){
                cmplCall("__main", NULL);
            }
            cmplStmt((Ast*)func->stmt, retLabel);
            // Return code
            emitLabelDecl(retLabel);
            emitIns0("leave");
            emitIns0("ret");
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
    maxLabelNum = 0;
    frameOffset = 0;
    emitAsm(".text\n");
    for (size_t i=0; i<top->globals.size; i++){
        cmplGlobal(top->globals.elem[i]);
    }
    disposeAddrTable();
}