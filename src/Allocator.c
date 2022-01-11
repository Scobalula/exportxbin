#include "Allocator.h"
#define ALLOCATOR_CHECKS 0
allocatedMemory_t* memory_root = NULL;
size_t alloced_slots = 0;

allocatedMemory_t* Allocator_CreateMemorySlot()
{
    if(memory_root == NULL)
    {
        memory_root = malloc(sizeof(allocatedMemory_t));
        memset(memory_root, 0, sizeof(allocatedMemory_t));
    }

    allocatedMemory_t* curr = memory_root;

    while(curr->next != NULL)
    {
        curr = curr->next;
    }

    curr->next = malloc(sizeof(allocatedMemory_t));
    memset(curr->next, 0, sizeof(allocatedMemory_t));
    curr->next->prev = curr;

    return curr->next;
}

allocatedMemory_t* Allocator_FindMemorySlot(void* ptr)
{
    if(memory_root == NULL)
    {
        memory_root = malloc(sizeof(allocatedMemory_t));
        memset(memory_root, 0, sizeof(allocatedMemory_t));
    }

    allocatedMemory_t* result = NULL;
    allocatedMemory_t* curr = memory_root;

    while(curr->next != NULL)
    {
        curr = curr->next;

        if(curr->ptr == ptr)
        {
            result = curr;
            break;
        }
    }

    return result;
}

void* Allocator_AllocateMemory(size_t size, const char* file, const char* func, int line)
{
    void* ptr = malloc(size);

#if ALLOCATOR_CHECKS
    allocatedMemory_t* slot = Allocator_CreateMemorySlot();

    slot->ptr = ptr;
    slot->size = size;
    slot->file = file;
    slot->func = func;
    slot->line = line;

    alloced_slots++;
#endif
    return ptr;
}

void* Allocator_ReallocateMemory(void* ptr, size_t size, const char* file, const char* func, int line)
{
    void* new_ptr = realloc(ptr, size);

    if(new_ptr == NULL)
    {
        return NULL;
    }

#if ALLOCATOR_CHECKS
    allocatedMemory_t* slot = Allocator_FindMemorySlot(ptr);

    if(slot == NULL)
    {
        printf(": ERROR: Passed memory we didn't allocate!");
        exit(-100000000);
    }

    slot->ptr = new_ptr;
    slot->size = size;
    slot->file = file;
    slot->func = func;
    slot->line = line;
#endif

    return new_ptr;
}

void Allocator_FreeMemory(void* ptr, const char* file, const char* func, int line)
{
#if ALLOCATOR_CHECKS
    if(ptr != NULL)
    {
        allocatedMemory_t* slot = Allocator_FindMemorySlot(ptr);

        if(slot == NULL)
        {
            printf(": ERROR: Passed memory we didn't allocate!");
            exit(-100000000);
        }

        allocatedMemory_t* prev = slot->prev;
        allocatedMemory_t* next = slot->next;

        if(next != NULL)
        {
            prev->next = next;
            next->prev = prev;
        }
        else
        {
            prev->next = NULL;
        }

        free(slot);

        alloced_slots--;
    }
#endif

    free(ptr);
}

void Allocator_Report(const char* file_name)
{
    if(memory_root == NULL)
    {
        return;
    }

    FILE* fp = NULL;

    if((fp = fopen(file_name, "w")) == NULL)
    {
        return;
    }

    allocatedMemory_t* curr = memory_root;

    while(curr->next != NULL)
    {
        curr = curr->next;

        fprintf(fp, "Allocated Ptr:\t0x%llx\n", (size_t)curr->ptr);
        fprintf(fp, "Allocated Size:\t0x%llx\n", (size_t)curr->size);
        fprintf(fp, "Allocated File:\t%s\n", curr->file);
        fprintf(fp, "Allocated Func:\t%s\n", curr->func);
        fprintf(fp, "Allocated Line:\t%i\n", curr->line);
    }
}