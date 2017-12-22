#include <stdlib.h>
#include <assert.h>
#include "./generic.h"

//Type macros required: KEY, VAL

#ifndef LOADFACTOR
#define LOADFACTOR 0.65f
#endif

#ifndef STATUSENUM
typedef enum {stEmpty=0, stDeleted=1, stFilled=2} StatusEnum;
#define STATUSENUM StatusEnum
#endif
#define Pair(k, v) CONCAT3(Pair, k, v)
typedef struct {
    KEY key;
    STATUSENUM status;
    VAL value;
} Pair(KEY, VAL);

#ifndef GETDATA
#define GETDATA(table) ((Pair(KEY, VAL)*)table->data)
#endif

//Inits map to all empty pairs of specific size.
char mapInit(KEY, VAL)(
    Map(KEY, VAL)* table,
    size_t initSize,
    size_t (*hash)(KEY),
    char (*eq)(KEY, KEY),
    void (*keyDtr)(KEY),
    void (*valDtr)(VAL)
){
    table->hash = hash;
    table->keyDtr = keyDtr;
    table->valDtr = valDtr;
    table->eq = eq;
    table->size = 0;
    if (initSize == 0){
        //Minimum size is 1 to prevent modulo arith exception when finding indexes
        initSize = 1;
    }
    size_t initAlloc = sizeof(Pair(KEY, VAL)) * initSize;
    table->data = malloc(initAlloc);
    if (table->data == NULL) return 0;
    for (size_t i=0; i<initSize; i++){
        GETDATA(table)[i].status = stEmpty;
    }
    table->allocatedSize = initSize;
    return 1;
}

void mapClear(KEY, VAL)(Map(KEY, VAL)* table){
    Pair(KEY, VAL)* pair;
    if (table->keyDtr){
        for (size_t i=0; i<table->allocatedSize; i++){
            pair = &GETDATA(table)[i];
            if (pair->status == stFilled){
                (*table->keyDtr)(pair->key);
            }
        }
    }
    if (table->valDtr){
        for (size_t i=0; i<table->allocatedSize; i++){
            pair = &GETDATA(table)[i];
            if (pair->status == stFilled){
                (*table->valDtr)(pair->value);
            }
        }
    }
    for (size_t i=0; i<table->allocatedSize; i++){
        GETDATA(table)[i].status = stEmpty;
    }
    table->size = 0;  
}

void mapDispose(KEY, VAL)(Map(KEY, VAL)* table){
    mapClear(KEY, VAL)(table);
    free(table->data);
}

#define toIndex(k, v) CONCAT3(toIndex, k, v)
//Hashes key and modulo it into valid index
static size_t toIndex(KEY, VAL)(const Map(KEY, VAL)* table, KEY key){
    return (*table->hash)(key) % (table->allocatedSize);
}

#define update(k, v) CONCAT3(update, k, v)
//Put key and value into table. If pair with same key is found then just edit it
static void update(KEY, VAL)(Map(KEY, VAL)* table, KEY key, VAL value){
    int pos = toIndex(KEY, VAL)(table, key);
    Pair(KEY, VAL)* pair;
    for (size_t i=pos; i<table->allocatedSize; i++){
        pair = &GETDATA(table)[i];
        if (pair->status != stFilled){
            table->size++;
            goto updatePair;
        }
        else if (table->eq(key, GETDATA(table)[i].key)){
            goto updateValueOnly;
        }
    }
    for (size_t i=0; i<pos; i++){
        pair = &GETDATA(table)[i];
        if (pair->status != stFilled){
            table->size++;
            goto updatePair;
        }
        else if (table->eq(key, GETDATA(table)[i].key)){
            goto updateValueOnly;
        }
    }
    assert(0 && "Map shouldn't be full");
    updatePair:
    pair->key = key;
    updateValueOnly:
    pair->value = value;
    pair->status = stFilled;
}

#define expand(k, v) CONCAT3(expand, k, v)
//Reallocate the entire table (double size), set the new data to NULL, and reupdates each old pair
//Returns whether reallocation succeeds
static char expand(KEY, VAL)(Map(KEY, VAL)* table){
    Pair(KEY, VAL)* oldData = table->data;
    size_t oldSize = table->allocatedSize;
    //Creates brand new hash table twice as big
    if (!mapInit(KEY, VAL)(table, table->allocatedSize * 2, table->hash, table->eq, table->keyDtr, table->valDtr)){
        free(oldData);
        return 0;
    }
    //Populate with old data pairs
    for (size_t i=0; i<oldSize; i++){
        if (oldData[i].status == stFilled){
            update(KEY, VAL)(table, oldData[i].key, oldData[i].value);
        }
    }
    free(oldData);
    return 1;
}

//Dynamic version of update that can expand table
char mapInsert(KEY, VAL)(Map(KEY, VAL)* table, KEY key, VAL value){
    //Reallocated if load on array is too high
    if (table->size > table->allocatedSize * LOADFACTOR) {
        //If reallocation fails then insertion fails
        if (!expand(KEY, VAL)(table)) return 0;
    }
    update(KEY, VAL)(table, key, value);
    return 1;
}

#define getProbe(k, v) CONCAT3(getProbe, k, v)
//Probes linearly from starting point. Stops at empty slot or filled slot with matching key
static Pair(KEY, VAL)* getProbe(KEY, VAL)(const Map(KEY, VAL)* table, size_t start, KEY key){
    Pair(KEY, VAL)* pair;
    for (size_t i=start; i<table->allocatedSize; i++){
        pair = &GETDATA(table)[i];
        if (pair->status == stEmpty){
            return NULL;
        }
        else if (pair->status == stFilled && table->eq(pair->key, key)){
            return pair;
        }
    }
    for (size_t i=0; i<start; i++){
        pair = &GETDATA(table)[i];
        if (pair->status == stEmpty){
            return NULL;
        }
        else if (pair->status == stFilled && table->eq(pair->key, key)){
            return pair;
        }
    }
    return NULL;
}

//Deletes entry from hash table if it exists. Returns pointer to value if successful otherwise none
//Will dispose the key but moves the value to the caller
const VAL* mapRemove(KEY, VAL)(Map(KEY, VAL)* table, KEY key){
    Pair(KEY, VAL)* pair = getProbe(KEY, VAL)(table, toIndex(KEY, VAL)(table, key), key);
    if (pair != NULL){
        pair->status = stDeleted;
        if (table->keyDtr) (*table->keyDtr)(pair->key);
        return &pair->value;
    }
    return NULL;
}

VAL* mapFind(KEY, VAL)(const Map(KEY, VAL)* table, KEY key){
    Pair(KEY, VAL)* pair = getProbe(KEY, VAL)(table, toIndex(KEY, VAL)(table, key), key);
    if (pair != NULL){
        return &pair->value;
    }
    return NULL;
}