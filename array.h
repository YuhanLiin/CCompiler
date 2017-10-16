#pragma once
#include "utils.h"

#define TYPE char_t
#include "generics/gen_array.h"
#undef TYPE 

typedef void* vptr; 
#define TYPE vptr
#include "generics/gen_array.h"
#undef TYPE 

#define TYPE size_t
#include "generics/gen_array.h"
#undef TYPE