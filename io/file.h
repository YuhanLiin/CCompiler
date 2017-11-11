#pragma once
#include <stdarg.h>
#include "utils.h"

void emitAsm(const char* format, ...);

char_t consumeNext();

void closeFiles(const char* infilename, const char* outfilename);

void openFiles(const char* infilename, const char* outfilename);
