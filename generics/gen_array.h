#include <stdlib.h>
#include <string.h>
#include "./generic.h"
//Requires definition of TYPE macro (one word type name) before using. Undef after

#ifndef Array
#define Array(t) CONCAT(Array, t)
#endif
typedef struct
{
    TYPE* elem;     //Actual array of elements. Part of interface 
    size_t size;    //# of elements in use. Part of interface
    size_t allocatedSize;   //# of elements allocated
    void (*destructor)(TYPE);     //Destructor function for each element. Can be null
} Array(TYPE);

#ifndef arrInit
#define arrInit(t) CONCAT(arrInit, t)
#endif
//Initialize flexible array using a c array, allocating and copying c array into new array
//If c array is null, then the memory is still allocated according to size but new array starts out empty 
//Thus, both arr and dtr can be null
char arrInit(TYPE) (Array(TYPE)* array, size_t size, const TYPE arr[], void (* dtr)(TYPE));

#ifndef arrClear
#define arrClear(t) CONCAT(arrClear, t)
#endif
//Destroys all elements and empties array
void arrClear(TYPE) (Array(TYPE)* array);

#ifndef arrDispose
#define arrDispose(t) CONCAT(arrDispose, t)
#endif
//Destroys all elements of array and array itself
void arrDispose(TYPE) (Array(TYPE)* array);

#ifndef arrInsert
#define arrInsert(t) CONCAT(arrInsert, t)
#endif
//Insert element into position and shift all elem after by 1. Reallocate twice the space if needed. 
//Returns success/fail
char arrInsert(TYPE) (Array(TYPE)* array, TYPE elem, size_t pos);

#ifndef arrPush
#define arrPush(t) CONCAT(arrPush, t)
#endif
//Push element into back of array 
char arrPush(TYPE) (Array(TYPE)* array, TYPE elem);

#ifndef arrExtract
#define arrExtract(t) CONCAT(arrExtract, t)
#endif
//Moves element from nth position and retract all following elems by 1. Returns that element without destroying it
TYPE arrExtract(TYPE) (Array(TYPE)* array, size_t pos);

#ifndef arrPop
#define arrPop(t) CONCAT(arrPop, t)
#endif
//Pop element from back and returns it. Does not call destructor
TYPE arrPop(TYPE) (Array(TYPE)* array);

#ifndef arrCopy
#define arrCopy(t) CONCAT(arrCopy, t)
#endif
//Copy one array's elements into another. Assumes dest array is unallocated or disposed (no remaining resources)
//Can fail due to malloc
char arrCopy(TYPE) (Array(TYPE)* dest, const Array(TYPE)* src);