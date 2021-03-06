#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "lexer/lexer.h"
#include "utils.h"
#include "ast/ast.h"
#include "scope/scope.h"
#include "semantics/symtable.h"
#include "codegen/asm_private.h"
#include "codegen/codegen.h"

//IMPORTANT Right now everything is in 64-bit mode, including constants. 
//This means upper bits will always be extended out so no danger for now. Type conversion will need to be handled when this ends

// # of labels used so far
static labelnum_t maxLabelNum = 0;
// Registers used as params in the MS x64 calling convention
static const Register paramRegisters[] = {$rcx, $rdx, $r8, $r9};
// r11 will be used as intermediate for all mov operations
static const Register movIntermediate = $r11;
// r10 will be used as intermediate for all binop operations
static const Register binopIntermediate = $r10;
// rax will be used as intermediate for all unop operations
static const Register unopIntermediate = $r10;
static const Register unopByteIntermediate = $r10b;

static void cmplMov(Address from, Address to){
    if (from.mode == indirectMode && to.mode == indirectMode){
        appendInstr(op2Instruction("movq", from, registerAddress(movIntermediate)));
        appendInstr(op2Instruction("movq", registerAddress(movIntermediate), to));
    }
    else{
        appendInstr(op2Instruction("movq", from, to));
    }
}

static void cmplStackPush(Address val, offset_t* frameOffset){
    *frameOffset -= 8;
    cmplMov(val, indirectAddress(*frameOffset, $rbp));
}

static Address cmplExpr(ExprBase* ast, offset_t* frameOffset, offset_t* maxCallSpace);

// Args array ptr can be null, signifying a call with no args
static void cmplCall(char_t* name, Array(vptr) *args, offset_t* frameOffset, offset_t* maxCallSpace){
    offset_t callSpace;
    if (args != NULL){
        // Process each argument from right to left
        for (int i=args->size-1; i>=0; i--){
            Address arg = cmplExpr((ExprBase*)args->elem[i], frameOffset, maxCallSpace);
            // Put the first 4 into registers
            if (i < 4){
                cmplMov(arg, registerAddress(paramRegisters[i]));
            }
            // The rest go onto stack right before the 32-bit shadow call space
            else{
                cmplMov(arg, indirectAddress(i * 8, $rsp));
            }
        }
        callSpace = args->size < 4 ? 32 : (args->size * 8);
    }
    else{
        callSpace = 32;
    }
    // Update max call space of function
    if (callSpace > *maxCallSpace) *maxCallSpace = callSpace; 
    appendInstr(op1Instruction("call", symbolAddress(name)));
}

static Address cmplUnop(ExprUnop* unop, offset_t* frameOffset, offset_t* maxCallSpace){
    Address addr = cmplExpr(unop->operand, frameOffset, maxCallSpace);
    // If it's a leftside increment/decrement, then just update and return the operand without a temp value
    if (unop->leftside){
        switch(unop->op){
            case tokInc:
                appendInstr(op1Instruction("incq", addr));
                return addr;
            case tokDec:
                appendInstr(op1Instruction("decq", addr));
                return addr;
        }
    }
    Address temp = registerAddress(unopIntermediate);
    cmplMov(addr, temp);
    switch(unop->op){
        case tokMinus:
            appendInstr(op1Instruction("negq", temp));
            break;
        case tokNot:
            appendInstr(op2Instruction("cmpq", numberAddress(0), temp));
            appendInstr(op1Instruction("sete", registerAddress(unopByteIntermediate)));
            appendInstr(op2Instruction("movzbq", registerAddress(unopByteIntermediate), registerAddress(unopIntermediate)));
            break;
        // Rightside increment and decrement
        case tokInc:
            appendInstr(op1Instruction("incq", addr));
            break;
        case tokDec:
            appendInstr(op1Instruction("decq", addr));
            break;
        default:
            assert(0 && "Not a token unary operator");
    }
    return temp;
}

//Multiply left by right and store in rax. Promotes result to type
static void cmplMulti(Address left, Address right, Type type){
    assert(left.mode == indirectMode && "Left operand must be on stack");
    //Put right operand on rax if its not already there
    if (right.mode != registerMode || right.val.reg != $rax){
        cmplMov(right, registerAddress($rax));
        right = registerAddress($rax);
    }
    if (isSignedType(type)){
        appendInstr(op1Instruction("imulq", left));
    }
    else{
        appendInstr(op1Instruction("mulq", left));
    }
}
static void cmplDiv(Address left, Address right, Type type){
    assert(left.mode == indirectMode && "Left operand must be on stack");
    //Can't run div instruction on a number. Also must have left operand on rax, so right operand can't be on rax
    if (right.mode == numberMode || (right.mode == registerMode && right.val.reg == $rax)){
        cmplMov(right, registerAddress(binopIntermediate));
        right = registerAddress(binopIntermediate);
    }
    cmplMov(left, registerAddress($rax));
    if (isSignedType(type)){
        appendInstr(op0Instruction("cqto"));
        appendInstr(op1Instruction("idivq", right));
    }
    else{
        cmplMov(numberAddress(0), registerAddress($rdx));
        appendInstr(op1Instruction("divq", right));
    }
}
// Perform specified arithemtic operation on operands and store value in left operand
static void cmplArith(const char_t* op, Address left, Address right, Type type){
    // Can't have both operands on stack, so move second one to intermediate register. 
    if (right.mode == indirectMode){
        cmplMov(right, registerAddress(binopIntermediate));
        right = registerAddress(binopIntermediate);
    }
    appendInstr(op2Instruction(op, right, left));
}
// Do relational comparison and store result in rax
static Address cmplRel(Token relOp, Address left, Address right, Type type){
    const char_t* opcode;
    switch(relOp){
        case tokEquals:
            opcode = "sete";
            break;
        case tokNotEquals:
            opcode = "setne";
            break;
        case tokGreaterEquals:
            if (isSignedType(type)) opcode = "setge";
            else opcode = "setae";
            break;
        case tokLessEquals:
            if (isSignedType(type)) opcode = "setle";
            else opcode = "setbe";
            break;
        case tokGreater:
            if (isSignedType(type)) opcode = "setg";
            else opcode = "seta";
            break;
        case tokLess:
            if (isSignedType(type)) opcode = "setl";
            else opcode = "setb";
            break;
    }
    cmplArith("cmpq", left, right, type);
    appendInstr(op1Instruction(opcode, registerAddress($al)));
    appendInstr(op2Instruction("movzbq", registerAddress($al), registerAddress($rax)));
    return registerAddress($rax);
}

// Move left operand onto stack and destructively operate on it
static Address cmplBinop(ExprBinop* binop, offset_t* frameOffset, offset_t* maxCallSpace){    
    Address left = cmplExpr(binop->left, frameOffset, maxCallSpace);
    // Move left operand onto the stack unless we're assigning to it directly
    if (!isAssignmentOp(binop->op)){
        cmplStackPush(left, frameOffset);
        left = indirectAddress(*frameOffset, $rbp);
    }
    Address right = cmplExpr(binop->right, frameOffset, maxCallSpace);
    
    // Left operand will always be the destination operand that is mutated
    switch(binop->op){
        case tokAssign:
            cmplArith("movq", left, right, binop->right->type);
            return left;
        case tokPlusAssign:
        case tokPlus:
            cmplArith("addq", left, right, binop->right->type);
            return left;
        case tokMinusAssign:
        case tokMinus:
            cmplArith("subq", left, right, binop->right->type);
            return left;
        case tokMulti:
            cmplMulti(left, right, binop->base.type);
            return registerAddress($rax);
        case tokMultiAssign:
            cmplMulti(left, right, binop->right->type);
            cmplMov(registerAddress($rax), left);
            return left;
        case tokDiv:
            cmplDiv(left, right, binop->base.type);
            return registerAddress($rax);
        case tokDivAssign:
            cmplDiv(left, right, binop->right->type);
            cmplMov(registerAddress($rax), left);
            return left;

        case tokEquals:
        case tokNotEquals:
        case tokGreaterEquals:
        case tokLessEquals:
        case tokGreater:
        case tokLess:
            return cmplRel(binop->op, left, right, binop->right->type);

        default:
            assert(0 && "Unhandled binop.");
    }
}

// Returns address of the result of the processed expression
static Address cmplExpr(ExprBase* expr, offset_t* frameOffset, offset_t* maxCallSpace){
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
            cmplCall(((ExprCall*)expr)->name, &((ExprCall*)expr)->args, frameOffset, maxCallSpace);
            return registerAddress($rax);
        case astExprIdent:
            return findAddress(((ExprIdent*)expr)->name);
        case astExprBinop: {
            return cmplBinop((ExprBinop*)expr, frameOffset, maxCallSpace);
        }
        case astExprUnop: 
            return cmplUnop((ExprUnop*)expr, frameOffset, maxCallSpace);
        default:
            assert(0 && "Unsupported AST for expr");
    }
}

typedef struct
{
    labelnum_t ret;
    labelnum_t brk;
    labelnum_t cont;
} LabelContext;

static void cmplStmt(Ast* ast, offset_t* frameOffset, offset_t* maxCallSpace, const LabelContext* labels){
    switch(ast->label){
        case astStmtEmpty:
            break;
        case astStmtExpr:
            cmplExpr(((StmtExpr*)ast)->expr, frameOffset, maxCallSpace);
            break;
        case astStmtReturn: {
            if (hasRetExpr((StmtReturn*)ast)){
                Address expaddr = cmplExpr((ExprBase*)((StmtReturn*)ast)->expr, frameOffset, maxCallSpace);
                cmplMov(expaddr, registerAddress($rax));
            }
            appendInstr(labelInstruction("jmp", numLabel(labels->ret)));
            break;
        }
        case astStmtBlock: {
            StmtBlock* blk = (StmtBlock*)ast;
            curScope = blk->scopeId;
            for (size_t i=0; i<blk->stmts.size; i++){
                cmplStmt(blk->stmts.elem[i], frameOffset, maxCallSpace, labels);
            }
            toPrevScope();
            break;
        }
        case astStmtDef: {
            StmtVar* def = (StmtVar*)ast;
            *frameOffset -= 8;
            if (def->rhs){
                cmplMov(cmplExpr(def->rhs, frameOffset, maxCallSpace), indirectAddress(*frameOffset, $rbp));
            }
            insertAddress(def->name, indirectAddress(*frameOffset, $rbp));
            break;
        }
        case astStmtWhile: {
            StmtWhileLoop* loop = (StmtWhileLoop*)ast;
            const LabelContext loopCtx = (LabelContext){.ret=labels->ret, .brk=maxLabelNum++, .cont=maxLabelNum++};

            //Evaluate condition after start label
            appendInstr(labelDeclInstruction(numLabel(loopCtx.cont)));
            Address cond = cmplExpr(loop->condition, frameOffset, maxCallSpace);
            if (cond.mode == numberMode){
                //If cond is 0 then skip everything, otherwise just loop back to start label infinitely
                if (cond.val.num != 0){
                    cmplStmt(loop->stmt, frameOffset, maxCallSpace, &loopCtx);
                    appendInstr(labelInstruction("jmp", numLabel(loopCtx.cont)));
                }
            }
            else{
                //If condition evaluates to 0 then exit loop via jump
                appendInstr(op2Instruction("cmpq", numberAddress(0), cond));
                appendInstr(labelInstruction("je", numLabel(loopCtx.brk)));
                //Otherwise evaluate inner statement then go back up
                cmplStmt(loop->stmt, frameOffset, maxCallSpace, &loopCtx);
                appendInstr(labelInstruction("jmp", numLabel(loopCtx.cont)));
            }
            appendInstr(labelDeclInstruction(numLabel(loopCtx.brk)));
            break;
        }
        case astStmtDoWhile: {
            StmtWhileLoop* loop = (StmtWhileLoop*)ast;
            size_t doStart = maxLabelNum++;
            const LabelContext doCtx = (LabelContext){.ret=labels->ret, .brk=maxLabelNum++, .cont=maxLabelNum++};

            //Evaluate statement then condition
            appendInstr(labelDeclInstruction(numLabel(doStart)));
            cmplStmt(loop->stmt, frameOffset, maxCallSpace, &doCtx);
            appendInstr(labelDeclInstruction(numLabel(doCtx.cont)));
            Address cond = cmplExpr(loop->condition, frameOffset, maxCallSpace);
            //Special treatment for const conditions
            if (cond.mode == numberMode){
                if (cond.val.num != 0){
                    appendInstr(labelInstruction("jmp", numLabel(doStart)));
                }
            }
            //If cond is not 0 jump back up and loop again
            else{
                appendInstr(op2Instruction("cmpq", numberAddress(0), cond));
                appendInstr(labelInstruction("jne", numLabel(doStart)));
            }
            appendInstr(labelDeclInstruction(numLabel(doCtx.brk)));
            break;
        }
        case astStmtContinue:
            appendInstr(labelInstruction("jmp", numLabel(labels->cont)));
            break;
        case astStmtBreak:
            appendInstr(labelInstruction("jmp", numLabel(labels->brk)));
            break;
        case astStmtIf: {
            StmtIf* ifelse = (StmtIf*)ast;
            size_t endLbl = maxLabelNum++;
            // No need to have separate else label if there's no else statement
            size_t elseLbl = ifelse->elseStmt ? maxLabelNum++ : endLbl;
            
            // If condition evaluates to 0 then jump to else branch (or end label if there's no else)
            Address cond = cmplExpr(ifelse->condition, frameOffset, maxCallSpace);
            appendInstr(op2Instruction("cmpq", numberAddress(0), cond));
            appendInstr(labelInstruction("je", numLabel(elseLbl)));
            // Otherwise execute the conditonal code
            cmplStmt(ifelse->ifStmt, frameOffset, maxCallSpace, labels);
            // If there is an else branch, the conditional code also needs to skip it and jmp to the end
            if (ifelse->elseStmt){
                appendInstr(labelInstruction("jmp", numLabel(endLbl)));
                appendInstr(labelDeclInstruction(numLabel(elseLbl)));
                cmplStmt(ifelse->elseStmt, frameOffset, maxCallSpace, labels);
            }
            appendInstr(labelDeclInstruction(numLabel(endLbl)));
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
            cmplMov(registerAddress(paramRegisters[i]), location);
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
            appendInstr(labelInstruction(".globl", strLabel(func->name)));
            appendInstr(labelDeclInstruction(strLabel(func->name)));
            appendInstr(op1Instruction("pushq", registerAddress($rbp)));
            cmplMov(registerAddress($rsp), registerAddress($rbp));

            // Create a new label for the return location
            const LabelContext lblctx = (LabelContext){.ret=maxLabelNum++, .brk=0, .cont=0};
            // Stack space needed for variables
            offset_t frameOffset = 0;
            // Stack space needed for function calls
            offset_t maxCallSpace = 0;

            // Set global scope variable so that symbol table can be used
            curScope = func->scopeId;
            cmplParams(&func->params);
            // Allocate space on stack for variables and calls. Amount allocated will be known later
            size_t rspInsIndex = appendInstr(op2Instruction("subq", numberAddress(0), registerAddress($rsp)));
            if (!strcmp(func->name, "main")){
                cmplCall("__main", NULL, &frameOffset, &maxCallSpace);
            }
            cmplStmt((Ast*)func->stmt, &frameOffset, &maxCallSpace, &lblctx);
            // Decide amount to allocate on stack
            assert(maxCallSpace >= 0 && frameOffset <= 0 && "Offset signs are wrong");
            getInstrPtr(rspInsIndex)->args.operands[0] = numberAddress(maxCallSpace - frameOffset);
            // Return routine
            appendInstr(labelDeclInstruction(numLabel(lblctx.ret)));
            appendInstr(op0Instruction("leave"));
            appendInstr(op0Instruction("ret"));
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
    appendInstr(op0Instruction(".text"));
    for (size_t i=0; i<top->globals.size; i++){
        cmplGlobal(top->globals.elem[i]);
    }
    disposeAddrTable();
}