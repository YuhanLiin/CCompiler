#pragma once
#include "utils.h"
#include <stdint.h>

typedef enum {
    $rbp, $rsp,
    $rax, $rcx, $rdx, $r8, $r9,
    $rbx, $rdi, $rsi, $r12, $r13, $r14, $r15
} Register;

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
        char_t* symbol;
        struct {Register reg; int64_t offset;} indirect;
    } val;
} Address;

Address registerAddress(Register reg);

Address symbolAddress(char_t *symbol);

Address numberAddress(uint64_t num);

Address indirectAddress(int64_t offset, Register reg);

void initAddrTable();

void disposeAddrTable();

void insertAddress(char_t* name, Address addr);

Address findAddress(char_t* name);