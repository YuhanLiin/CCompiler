#pragma once

typedef enum {
    typNone,    //Sentinel for a failed type parse
    typInt32,
    typInt64,
    typFloat32,
    typFloat64
} Type;