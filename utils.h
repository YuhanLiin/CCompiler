#pragma once
#include <stdlib.h>

//Allocate new T ptr variable "name" for n instances of T. Exit if malloc fails. Used for all malloc cases. Treat as statement
#define New(T, name, n) T* name = malloc(sizeof(T)*n); if (name==NULL){exit(1);}

#define char_t char
#if char_t == wchar_t
//TODO substitute char functions with wchar equivalents
#endif