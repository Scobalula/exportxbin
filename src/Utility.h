#ifndef UTILITY_INCLUDED
#define UTILITY_INCLUDED
#include "Allocator.h"

#define malloc(X) Allocator_AllocateMemory(X, __FILE__, __FUNCTION__, __LINE__)
#define realloc(X, Y) Allocator_ReallocateMemory(X, Y, __FILE__, __FUNCTION__, __LINE__)
#define free(X) Allocator_FreeMemory(X, __FILE__, __FUNCTION__, __LINE__)

#endif