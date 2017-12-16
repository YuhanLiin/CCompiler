#include "codegen/asm_private.h"
#include "utils.h"
#include "io/file.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define TYPE AsmInstruction
#include "generics/gen_array.h"
#include "generics/gen_array.c"
#undef TYPE
Array(AsmInstruction) instructionBuffer;

size_t appendInstr(AsmInstruction ins){
    if (!arrPush(AsmInstruction)(&instructionBuffer, ins)) exit(1);
    return instructionBuffer.size - 1;
}

AsmInstruction* getInstrPtr(size_t i){
    return &instructionBuffer.elem[i];
}

void initAsm(){
    if(!arrInit(AsmInstruction)(&instructionBuffer, 10, NULL, NULL)) exit(1);
}
void disposeAsm(){
    arrDispose(AsmInstruction)(&instructionBuffer);
}

//Below code has to do with emiting instructions

// Never use this as a format string
const char_t* registerStr(Register reg){
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

static void emitAddr(const Address* addr){
    switch (addr->mode){
        case registerMode:
            emitOut("%s", registerStr(addr->val.reg));
            break;
        case symbolMode:
            emitOut(addr->val.symbol);
            break;
        case numberMode:
            emitOut("$%llu", addr->val.num);
            break;
        case indirectMode:
            emitOut("%lld(%s)", addr->val.indirect.offset, registerStr(addr->val.indirect.reg));
            break;
        default:
            assert(0 && "Unsupported addressing mode");
    }
}

static void emitLabel(const Label* label){
    switch(label->type){
        case lblNum:
            emitOut(".L%llu", label->data.num);
            break;
        case lblStr:
            emitOut("%s", label->data.str);
            break;
        default:
            assert(0 && "Unsupported label type");
    }
}

void emitInstr(const AsmInstruction* ins){
    switch(ins->type){
        case ins0Op:
            emitOut("\t%s", ins->opcode);
            break;
        case ins1Op:
            emitOut("\t%s ", ins->opcode);
            emitAddr(&ins->args.operands[0]);
            break;
        case ins2Op:
            emitOut("\t%s ", ins->opcode);
            emitAddr(&ins->args.operands[0]);
            emitOut(", ");
            emitAddr(&ins->args.operands[1]);
            break;
        case insLbl:
            emitOut("\t%s ", ins->opcode);
            emitLabel(&ins->args.label);
            break;
        case insLblDecl:
            emitLabel(&ins->args.label);
            emitOut(":");
            break;
        default:
            assert(0 && "Unsupported instruction type");
    }
    emitOut("\n");
}

void emitAllAsm(){
    for (size_t i=0; i<instructionBuffer.size; i++){
        emitInstr(&instructionBuffer.elem[i]);
    }
}