#ifndef ALLOCATOR_INCLUDED
#define ALLOCATOR_INCLUDED
#include "Common.h"

typedef struct allocatedMemory
{
    struct allocatedMemory* prev;
    struct allocatedMemory* next;
    void* ptr;
    size_t size;
    const char* file;
    const char* func;
    int line;
} allocatedMemory_t;

allocatedMemory_t* Allocator_CreateMemorySlot();

allocatedMemory_t* Allocator_FindMemorySlot(void* ptr);

void* Allocator_AllocateMemory(size_t size, const char* file, const char* func, int line);

void* Allocator_ReallocateMemory(void* ptr, size_t size, const char* file, const char* func, int line);

void Allocator_FreeMemory(void* ptr, const char* file, const char* func, int line);

void Allocator_Report(const char* file_name);
#endif