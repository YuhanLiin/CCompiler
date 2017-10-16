#pragma once

typedef enum {
    typNone,    //Sentinel for a failed type parse
    typInt32,
    typInt64,
    typFloat32,
    typFloat64
} Type;

typedef struct {
    char* name;
    size_t scopeId;
} Symbol;

#define GLOBAL_SCOPE 0;

