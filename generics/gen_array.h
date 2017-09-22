#include <stdlib.h>
#include <string.h>
#include "generic.h"
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

#ifndef initArr
#define initArr(t) CONCAT(initArr, t)
#endif
//Initialize flexible array using a c array, allocating and copying c array into new array
//If c array is null, then the memory is still allocated according to size but new array starts out empty 
//Thus, both arr and dtr can be null
char initArr(TYPE) (Array(TYPE)* array, size_t size, const TYPE arr[], void (* dtr)(TYPE));

#ifndef clearArr
#define clearArr(t) CONCAT(clearArr, t)
#endif
//Destroys all elements and empties array
void clearArr(TYPE) (Array(TYPE)* array);

#ifndef disposeArr
#define disposeArr(t) CONCAT(disposeArr, t)
#endif
//Destroys all elements of array and array itself
void disposeArr(TYPE) (Array(TYPE)* array);

#ifndef insertArr
#define insertArr(t) CONCAT(insertArr, t)
#endif
//Insert element into position and shift all elem after by 1. Reallocate twice the space if needed. 
//Returns success/fail
char insertArr(TYPE) (Array(TYPE)* array, TYPE elem, size_t pos);

#ifndef pushArr
#define pushArr(t) CONCAT(pushArr, t)
#endif
//Push element into back of array 
char pushArr(TYPE) (Array(TYPE)* array, TYPE elem);

#ifndef extractArr
#define extractArr(t) CONCAT(extractArr, t)
#endif
//Moves element from nth position and retract all following elems by 1. Returns that element without destroying it
TYPE extractArr(TYPE) (Array(TYPE)* array, size_t pos);

#ifndef popArr
#define popArr(t) CONCAT(popArr, t)
#endif
//Pop element from back and returns it. Does not call destructor
TYPE popArr(TYPE) (Array(TYPE)* array);

#ifndef copyArr
#define copyArr(t) CONCAT(copyArr, t)
#endif
//Copy one array's elements into another. Assumes dest array is unallocated or disposed (no remaining resources)
//Can fail due to malloc
char copyArr(TYPE) (Array(TYPE)* dest, const Array(TYPE)* src);