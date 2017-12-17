// Private headers for codegen
#pragma once
#include "utils.h"
#include <stdint.h>

typedef enum {
    $rbp, $rsp,
    $al, $r10b, $r11b,
    $rax, $rcx, $rdx, $r8, $r9, $r10, $r11
    // ,$rbx, $rdi, $rsi, $r12, $r13, $r14, $r15
} Register;

typedef int64_t offset_t;
typedef uint64_t labelnum_t;
typedef struct {
    enum {
        registerMode, 
        symbolMode, 
        numberMode, 
        indirectMode
    } mode;
    union {
        Register reg;
        uint64_t num;
        const char_t* symbol;
        struct {Register reg; offset_t offset;} indirect;
    } val;
} Address;

#define registerAddress(regst) (Address){registerMode, {.reg = regst}}
#define symbolAddress(sym) (Address){symbolMode, {.symbol = sym}}
#define numberAddress(number) (Address){numberMode, {.num = number}}
#define indirectAddress(off, regst) (Address){indirectMode, {.indirect = {.offset = off, .reg = regst}}}

typedef struct {
    enum {
        lblStr,
        lblNum
    } type;
    union {
        const char_t* str;
        labelnum_t num;
    } data;
} Label;

#define numLabel(number) (Label){lblNum, {.num = number}}
#define strLabel(string) (Label){lblStr, {.str = string}}

typedef struct {
    enum {
        ins0Op,
        ins1Op,
        ins2Op,
        insLbl,
        insLblDecl
    } type;
    const char_t* opcode;
    union {
        Address operands[2];
        Label label;
    } args;
} AsmInstruction;

#define op0Instruction(opcode) (AsmInstruction){ins0Op, opcode}
#define op1Instruction(opcode, op1) (AsmInstruction){ins1Op, opcode, {.operands = {op1}}}
#define op2Instruction(opcode, op1, op2) (AsmInstruction){ins2Op, opcode, {.operands = {op1, op2}}}
#define labelInstruction(opcode, lbl) (AsmInstruction){insLbl, opcode, {.label = lbl}}
#define labelDeclInstruction(lbl) (AsmInstruction){insLblDecl, NULL, {.label = lbl}}

size_t appendInstr(AsmInstruction ins);
AsmInstruction* getInstrPtr(size_t i);

void initAddrTable();
void disposeAddrTable();

void insertAddress(char_t* name, Address addr);
Address findAddress(char_t* name);

const char_t* registerStr(Register);
void emitInstr(const AsmInstruction*);