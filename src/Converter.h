#ifndef CONVERTER_INCLUDED
#define CONVERTER_INCLUDED
#include "Globals.h"
#include "Utility.h"
#include "Common.h"

enum
{
    CONVERT_TYPE_BIN_TO_EXPORT,
    CONVERT_TYPE_EXPORT_TO_BIN,
};

typedef struct converterItem
{
    pthread_t thread;
    char* file_path;
    int convert_type;
} converterItem_t;

int Converter_Init(size_t thread_count);

int Converter_AddItem(const char* file_path, int convert_type);

int Converter_Run();

int Converter_Shutdown();

size_t Converter_NumFilesProcessed();

#endif