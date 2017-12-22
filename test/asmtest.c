#include "codegen/asm_private.h"
#include "codegen/codegen.h"

#include "test/utils/io.c"
#include "test/utils/assert.h"

static void ts(AsmInstruction instr, const char* expected) {
    ioSetup("");
    emitInstr(&instr);
    assertEqStr(output, expected);
}

static void testEmitLabel(){
    ts(labelInstruction("jmp", numLabel(3)), "\tjmp .L3\n");
    ts(labelInstruction(".globl", strLabel("main")), "\t.globl main\n");
    ts(labelDeclInstruction(numLabel(222)), ".L222:\n");
}

static void testEmitOperations(){
    ts(op0Instruction("cqlt"), "\tcqlt\n");
    ts(op1Instruction("pushq", numberAddress(11111)), "\tpushq $11111\n");
    ts(op1Instruction("divq", registerAddress($rsp)), "\tdivq %rsp\n");
    ts(op2Instruction("addq", symbolAddress("x"), indirectAddress(-16, $rax)), "\taddq x, -16(%rax)\n");
}

static void testEmitAll(){
    initAsm();
    for (size_t i=1; i<5; i++){
        appendInstr(labelInstruction("je", numLabel(i)));
    }
    ioSetup("");
    emitAllAsm();
    assertEqStr(output, "\tje .L1\n\tje .L2\n\tje .L3\n\tje .L4\n");
    disposeAsm();
}

static void testRegStr(){
    for (Register reg=$rbp; reg<=$r11; reg++){
        registerStr(reg);
    }
}

int main(int argc, char const *argv[])
{
    testEmitLabel();
    testEmitOperations();
    testEmitAll();
    testRegStr();
    return 0;
}