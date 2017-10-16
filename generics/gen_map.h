#include "generic.h"

#define Map(k, v) CONCAT3(Map, k, v)
typedef struct {
    size_t (*hash)(KEY);
    void (*keyDtr)(KEY);
    void (*valDtr)(VAL);
    char (*eq)(KEY, KEY);
    size_t size;    //Total of filled and deleted elements 
    size_t allocatedSize;
    void* data;
} Map(KEY, VAL);

#define mapInit(k, v) CONCAT3(mapInit, k, v)
char mapInit(KEY, VAL)(Map(KEY, VAL)* table, size_t initSize, size_t (*hash)(KEY), char (*eq)(KEY, KEY), void (*keyDtr)(KEY), void (*valDtr)(VAL));

#define mapClear(k, v) CONCAT3(mapClear, k, v)
void mapClear(KEY, VAL)(Map(KEY, VAL)* table);

#define mapDispose(k, v) CONCAT3(mapDispose, k, v)
void mapDispose(KEY, VAL)(Map(KEY, VAL)* table);

#define mapInsert(k, v) CONCAT3(mapInsert, k, v)
char mapInsert(KEY, VAL)(Map(KEY, VAL)* table, KEY key, VAL value);

#define mapRemove(k, v) CONCAT3(mapRemove, k, v)
const VAL* mapRemove(KEY, VAL)(Map(KEY, VAL)* table, KEY key, KEY* keyReturn);

#define mapFind(k, v) CONCAT3(mapFind, k, v)
VAL* mapFind(KEY, VAL)(const Map(KEY, VAL)* table, KEY key);
