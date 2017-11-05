#pragma once
#include "utils.h"

typedef enum {$rbp, $rsp, $rax, $rcx, $rdx, $r8, $r9} Register;

typedef struct {
    enum {
        registerMode, 
        symbolMode, 
        numberMode, 
        indirectMode
    } mode;
    union {
        Register reg;
        long long int num;
        char_t* symbol;
        struct {Register reg; long long int offset;} indirect;
    } val;
} Address;

Address registerAddress(Register reg);

Address symbolAddress(char_t *symbol);

Address numberAddress(long long int num);

Address indirectAddress(long long int offset, Register reg);

void initAddrTable();

void disposeAddrTable();

void insertAddress(char_t* name, Address addr);

Address findAddress(char_t* name);