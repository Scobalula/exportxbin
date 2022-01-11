#include "Converter.h"
#include "MemoryFile.h"
#include "FileSystem.h"
#include "FileName.h"
#include "ExportFile.h"
#include "BinaryFile.h"
#include "Token.h"
#include <Windows.h>

size_t max_capacity = 4;
size_t cur_item_index = 0;
size_t num_files_processed = 0;
converterItem_t* items = NULL;

void* ProcessBinFile(void* arg)
{
    const char* file_path = (const char*)arg;

    tokenData_t data;
    memset(&data, 0, sizeof(data));
    memoryFile_t file;
    memset(&file, 0, sizeof(file));

    char* new_file_path = NULL;
    FILE* fp = NULL;

    if(FileName_HasExtension(file_path, ".xmodel_bin"))
    {
        new_file_path = FileName_ChangeExtension(file_path, ".xmodel_export");
    }
    else
    {
        new_file_path = FileName_ChangeExtension(file_path, ".xanim_export");
    }

    char* cur_file_name = FileName_GetFileName(file_path);
    char* new_file_name = FileName_GetFileName(new_file_path);

    printf("| Converting: %s to %s...\n", cur_file_name, new_file_name);

    int result = BinaryFile_Open(file_path, &file, 0);

    if(result == -1)
    {
        printf("| ERROR %s: Failed to open binary file.\n", cur_file_name);
        MemoryFile_Close(&file);
    }
    else if(result == -2)
    {
        printf("| ERROR: Failed to read binary file %s.\n", cur_file_name);
        MemoryFile_Close(&file);
    }
    else if(result != 1)
    {
        printf("| ERROR: Failed to decompress binary file %s, return: %i\n", cur_file_name, result);
        MemoryFile_Close(&file);
    }

    if(result == 1)
    {
        if((fp = fopen(new_file_path, "w")) == NULL)
        {
            printf("| ERROR: Failed to create export file.\n");
            result = 0;
        }
    }

    if(result == 1)
    {
        while(file.pos < file.size)
        {
            int result = BinaryReader_ReadToken(&file, &data);

            if(result == 0)
            {
                printf("| Error: Hit end of binary file %s.\n", cur_file_name);
                break;
            }
            else if(result == -1)
            {
                printf("| Error: Invalid binary token hit in %s at 0x%llx.\n", cur_file_name, file.pos);
                break;
            }
            else if(result == -1)
            {
                printf("| Error: Failed binary to read data in %s at 0x%llx.\n", cur_file_name, file.pos);
                break;
            }

            int write_result = ExportFile_WriteToken(fp, &data);
            Token_FreeData(&data);

            if(write_result <= 0)
            {
                printf("| Error %s: Failed to write to export file.\n", cur_file_name);
                result = 0;
                break;         
            }
        }
    }

    if(result != 0)
    {
        printf("| Converted: %s to %s.\n", cur_file_name, new_file_name);
    }

    if(fp != NULL)
    {
        fclose(fp);
    }

    MemoryFile_Close(&file);
    Token_FreeData(&data);
    FileName_FreeResult(cur_file_name);
    FileName_FreeResult(new_file_name);
    FileName_FreeResult(new_file_path);

    pthread_exit(NULL);
}

void* ProcessExportFile(void* arg)
{
    const char* file_path = (const char*)arg;

    tokenData_t data;
    memset(&data, 0, sizeof(data));
    memoryFile_t file;
    memset(&file, 0, sizeof(file));

    FILE* fp;

    char* new_file_path = NULL;

    if(FileName_HasExtension(file_path, ".xmodel_export"))
    {
        new_file_path = FileName_ChangeExtension(file_path, ".xmodel_bin");
    }
    else
    {
        new_file_path = FileName_ChangeExtension(file_path, ".xanim_bin");
    }

    char* cur_file_name = FileName_GetFileName(file_path);
    char* new_file_name = FileName_GetFileName(new_file_path);

    printf("| Converting: %s to %s...\n", cur_file_name, new_file_name);

    int valid = 1;

    if(MemoryFile_ConsumeFromFile(file_path, &file) != MEMORYFILE_OK)
    {
        printf("| ERROR %s: Failed to consume file.", cur_file_name);
        valid = 0;
    }

    file.name = cur_file_name;

    memoryFile_t bin_file;

    MemoryFile_InitBlank(&bin_file, file.size);

    if(valid)
    {
        while(file.pos < file.size)
        {
            int result = ExportFile_ReadToken(&file, &data);

            if(result == 0)
            {
                printf("| ERROR: Failed to read data in file %s at 0x%llx.\n", file.name, file.pos);
                valid = 0;
                break;
            }
            // EOF, not an error per say in a text file
            if(result == -1)
            {
                break;
            }

            BinaryFile_WriteToken(&bin_file, &data);
            Token_FreeData(&data);
        }
    }
    
    if(valid)
    {
        if(BinaryFile_Save(new_file_path, &bin_file, 0))
        {
            printf("| Converted: %s to %s...\n", cur_file_name, new_file_name);   
        }
        else
        {
            printf("| ERROR: Failed to save %s (File/Compression Error).", new_file_name);
        }
    }

    MemoryFile_Close(&file);
    MemoryFile_Close(&bin_file);
    Token_FreeData(&data);
    FileName_FreeResult(cur_file_name);
    FileName_FreeResult(new_file_name);
    FileName_FreeResult(new_file_path);

    pthread_exit(NULL);
}

int Converter_Init(size_t thread_count)
{
    if(thread_count == 0)
    {
        // TODO: Cross platform
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        thread_count = (size_t)sysinfo.dwNumberOfProcessors;
    }
    items = malloc(thread_count * sizeof(converterItem_t));
    memset(items, 0, thread_count * sizeof(converterItem_t));
    max_capacity = thread_count;
    cur_item_index = 0;
}

int Converter_AddItem(const char* file_path, int convert_type)
{
    size_t str_len = strlen(file_path);
    converterItem_t* item = &items[cur_item_index++];
    memset(item, 0, sizeof(converterItem_t));

    item->file_path = malloc(str_len + 1);
    item->convert_type = convert_type;

    memcpy(item->file_path, file_path, str_len);
    item->file_path[str_len] = '\0';

    // If we're at capacity, we should run what we have
    // and clear our slots
    if(cur_item_index >= max_capacity)
    {
        cur_item_index = 0;

        if(!Converter_Run())
        {
            return 0;
        }
    }

    return 1;
}

int Converter_Run()
{
    if(items == NULL)
    {
        return 0;
    }

    if(max_capacity == 0)
    {
        return 0;
    }

    for(size_t i = 0; i < max_capacity; i++)
    {
        if(items[i].file_path != NULL)
        {
            switch (items[i].convert_type)
            {
            case 0:
                pthread_create(&items[i].thread, NULL, &ProcessBinFile, (void*)items[i].file_path);
                break;
            case 1:
                pthread_create(&items[i].thread, NULL, &ProcessExportFile, (void*)items[i].file_path);
                break;
            default:
                break;
            }

            num_files_processed++;
        }
    }

    for(size_t i = 0; i < max_capacity; i++)
    {
        if(items[i].file_path != NULL)
        {
            void* res;
            int r_val = pthread_join(items[i].thread, &res);

            if (r_val != 0)
            {
                printf("| FATAL: Failed to join thread.\n");
                exit(-2000);
            }

            free(items[i].file_path);
        }
    }

    memset(items, 0, max_capacity * sizeof(converterItem_t));

    cur_item_index = 0;

    return 1;
}

int Converter_Shutdown()
{
    if(items != NULL)
    {
        for(size_t i = 0; i < max_capacity; i++)
        {
            if(items[i].file_path != NULL)
            {
                free(items[i].file_path);
            }
        }

        free(items);
    }

    return 1;
}

size_t Converter_NumFilesProcessed()
{
    return num_files_processed;
}