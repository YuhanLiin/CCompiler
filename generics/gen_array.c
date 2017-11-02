#include <stdlib.h>
#include <string.h>
//Requires definition of TYPE macro (one word type name) before using. Undef after



//Initialize flexible array using a c array, allocating and copying c array into new array
//If c array is null, then the memory is still allocated according to size but new array starts out empty 
//Thus, both arr and dtr can be null. On malloc failure, all fields will be NULL or 0
char arrInit(TYPE) (Array(TYPE)* array, size_t size, const TYPE arr[], void (* dtr)(TYPE)){
    array->size = 0;
    array->allocatedSize = 0; //Initialize size fields as 0
    array->destructor = dtr;
    if (size == 0){ //If initial size is empty then dont allocate memory, since its not needed
        array->elem = NULL;
    }
    else{
        size_t mem = sizeof(TYPE)*size;
        array->size = size;
        array->elem = malloc(mem);
        if (array->elem == NULL) return 0;
        array->allocatedSize = size; // If allocation succeeds update allocation size field
        //If arr isnt null then update size field and copy arr into the new array
        if (arr != NULL){  
            memcpy(array->elem, arr, mem);
        }
    }
    return 1;
}

//Destroys all elements and empties array
void arrClear(TYPE) (Array(TYPE)* array){
    if (array->destructor != NULL){
        for (size_t i=0; i<array->size; i++){
            (*array->destructor)(array->elem[i]);
        }
    }
    array->size = 0;
}

//Destroys all elements of array and array itself
void arrDispose(TYPE) (Array(TYPE)* array){
    arrClear(TYPE) (array);
    free(array->elem);
}

//Insert element into position and shift all elem after by 1. Reallocate twice the space if needed. 
//Returns success/fail
char arrInsert(TYPE) (Array(TYPE)* array, TYPE elem, size_t pos){
    if (array->size >= array->allocatedSize){
        size_t newSize = array->allocatedSize * 2 + 4;
        TYPE* newMem = realloc(array->elem, sizeof(TYPE)*newSize);
        if (newMem == NULL) return 0;
        array->elem = newMem;
        array->allocatedSize = newSize;  
    }
    for (size_t i=array->size; i>pos; i--){
        array->elem[i] = array->elem[i-1];
    }
    array->elem[pos] = elem;
    array->size++;
    return 1;
}
//Push element into back of array 
char arrPush(TYPE) (Array(TYPE)* array, TYPE elem){
    return arrInsert(TYPE) (array, elem, array->size);
}

//Moves element from nth position and retract all following elems by 1. Returns that element without destroying it
TYPE arrExtract(TYPE) (Array(TYPE)* array, size_t pos){
    TYPE elem = array->elem[pos];
    for (size_t i=pos+1; i<array->size; i++){
        array->elem[i-1] = array->elem[i];
    }
    array->size--;
    return elem;
}
//Pop element from back and returns it. Does not call destructor
TYPE arrPop(TYPE) (Array(TYPE)* array){
    return arrExtract(TYPE) (array, array->size-1);
}

//Copy one array's elements and destructor into another. Assumes dest array is unallocated or disposed (no remaining resources)
//Can fail due to malloc
char arrCopy(TYPE) (Array(TYPE)* dest, const Array(TYPE)* src){
    return arrInit(TYPE)(dest, src->size, src->elem, src->destructor);
}