#pragma once
#include "utils.h"
#include "semantic.h"

#define TYPE char_t
#include "generics/gen_array.h"
#undef TYPE 

typedef void* vptr; 
#define TYPE vptr
#include "generics/gen_array.h"
#undef TYPE 

#define TYPE Type
#include "generics/gen_array.h"
#undef TYPE