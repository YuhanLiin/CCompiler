#include "array.h"

#define TYPE char_t
#include "generics/gen_array.c"
#undef TYPE 

#define TYPE vptr
#include "generics/gen_array.c"
#undef TYPE 

#define TYPE Type
#include "generics/gen_array.c"
#undef TYPE