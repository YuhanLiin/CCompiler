#include <stdio.h>
#include <stdarg.h>
#include "utils.h"

void writeError(size_t line, size_t pos, char_t* message, ...){
    fprintf(stderr, "On line %d, position %d, ", line, pos);
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}