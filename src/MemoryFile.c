#include "Utility.h"
#include "Common.h"
#include "MemoryFile.h"
#include <Windows.h>

int MemoryFile_InitBlank(memoryFile_t* file, unsigned long long initial_buffer_size)
{
    memset(file, 0, sizeof(file));

    file->size = 0;
    file->capacity = initial_buffer_size;
    file->pos = 0;

    if(file->capacity > 0)
    {
        file->data = malloc(file->capacity);
    }
}

int MemoryFile_ReallocIfNeeded(memoryFile_t* file, size_t size)
{
    if((file->pos + size) > file->capacity)
    {
        if(file->capacity == 0)
        {
            file->capacity = size;
        }
        else
        {
            file->capacity *= 2;
        }

        void* new_buffer = realloc(file->data, file->capacity);

        if(new_buffer == NULL)
        {
            return 0;
        }

        file->data = new_buffer;
        file->size = file->pos + size;
    }

    return 1;
}

int MemoryFile_ConsumeFromFile(const char* name, memoryFile_t* file)
{
    FILE* fp;

    if((fp = fopen(name, "rb")) == NULL)
    {
        return -1;
    }

    _fseeki64(fp, 0L, SEEK_END);
    long long sizeOfFile = _ftelli64(fp);
    _fseeki64(fp, 0L, SEEK_SET);

    file->data = malloc(sizeOfFile);
    file->size = sizeOfFile;
    file->capacity = sizeOfFile;
    file->pos = 0;

    size_t consumed = fread(file->data, sizeof(char), file->size, fp);

    fclose(fp);

    return 0;
}

size_t MemoryFile_Seek(memoryFile_t* file, size_t position, int mode)
{
    return 0;
}

size_t MemoryFile_Align(memoryFile_t* file, size_t alignment)
{
    size_t position = file->pos;
    size_t result = position & ~(alignment - 1);

    if (result != position)
    {
        file->pos = result + alignment;
    }

    return file->pos;
}

int MemoryFile_Read(memoryFile_t* file, void* v, size_t size_of_data)
{
    if((file->pos + size_of_data) > file->size)
    {
        return MEMORYFILE_PASTEND;
    }
    
    memcpy(v, &file->data[file->pos], size_of_data);
    file->pos += size_of_data;
    return 0;
}

int MemoryFile_ReadChar(memoryFile_t* file, char* v)
{
    if((file->pos + sizeof(char)) > file->size)
    {
        return -1;
    }
    
    *v = *(char*)&file->data[file->pos];
    file->pos += sizeof(char);
    return 0;
}

int MemoryFile_ReadInt8(memoryFile_t* file, signed char* v)
{
    if((file->pos + sizeof(signed char)) > file->size)
    {
        return -1;
    }
    
    *v = *(signed char*)&file->data[file->pos];
    file->pos += sizeof(signed char);
    return 0;
}

int MemoryFile_ReadInt16(memoryFile_t* file, short* v)
{
    if((file->pos + sizeof(short)) > file->size)
    {
        return -1;
    }
    
    *v = *(short*)&file->data[file->pos];
    file->pos += sizeof(short);
    return 0;
}

int MemoryFile_ReadInt32(memoryFile_t* file, int* v)
{
    if((file->pos + sizeof(int)) > file->size)
    {
        return -1;
    }
    
    *v = *(int*)&file->data[file->pos];
    file->pos += sizeof(int);
    return 0;
}

int MemoryFile_ReadInt64(memoryFile_t* file, long long* v)
{
    if((file->pos + sizeof(long long)) > file->size)
    {
        return -1;
    }
    
    *v = *(long long*)&file->data[file->pos];
    file->pos += sizeof(long long);
    return 0;
}

int MemoryFile_ReadUInt8(memoryFile_t* file, unsigned char* v)
{
    if((file->pos + sizeof(unsigned char)) > file->size)
    {
        return -1;
    }
    
    *v = *(unsigned char*)&file->data[file->pos];
    file->pos += sizeof(unsigned char);
    return 0;
}

int MemoryFile_ReadUInt16(memoryFile_t* file, unsigned short* v)
{
    if((file->pos + sizeof(unsigned short)) > file->size)
    {
        return -1;
    }
    
    *v = *(unsigned short*)&file->data[file->pos];
    file->pos += sizeof(unsigned short);
    return 0;
}

int MemoryFile_ReadUInt32(memoryFile_t* file, unsigned int* v)
{
    if((file->pos + sizeof(unsigned int)) > file->size)
    {
        return -1;
    }
    
    *v = *(unsigned int*)&file->data[file->pos];
    file->pos += sizeof(unsigned int);
    return 0;
}

int MemoryFile_ReadUInt64(memoryFile_t* file, unsigned long long* v)
{
    if((file->pos + sizeof(unsigned long long)) > file->size)
    {
        return -1;
    }
    
    *v = *(unsigned long long*)&file->data[file->pos];
    file->pos += sizeof(unsigned long long);
    return 0;
}

int MemoryFile_ReadFloat(memoryFile_t* file, float* v)
{
    if((file->pos + sizeof(float)) > file->size)
    {
        return -1;
    }
    
    *v = *(float*)&file->data[file->pos];
    file->pos += sizeof(float);
    return 0;
}

int MemoryFile_ReadDouble(memoryFile_t* file, double* v)
{
    if((file->pos + sizeof(double)) > file->size)
    {
        return -1;
    }
    
    *v = *(double*)&file->data[file->pos];
    file->pos += sizeof(double);
    return 0;
}

int MemoryFile_ReadUTF8NullTerminatedString(memoryFile_t* file, char** v, size_t* size_result)
{
    size_t i = 0;
    size_t size = MEMORYFILE_DEFAULT_STR_SIZE;
    char* result = malloc(size);

    while(1)
    {
        if(file->pos > file->size)
        {
            free(result);
            return MEMORYFILE_PASTEND;
        }

        char c = file->data[file->pos++];

        if(i >= size)
        {
            size += MEMORYFILE_DEFAULT_STR_SIZE;
            char* new_buffer = realloc(result, size);

            if(new_buffer == NULL)
            {
                free(result);
                return MEMORYFILE_INVALID;
            }

            result = new_buffer;
        }

        result[i++] = c;

        if(c == '\0')
        {
            break;
        }
    }

    char* final = realloc(result, i);

    if(final == NULL)
    {
        free(result);
        return MEMORYFILE_INVALID;
    }

    *v = final;

    if(size_result != NULL)
    {
        *size_result = i;
    }

    return MEMORYFILE_OK;
}

int MemoryFile_Write(memoryFile_t* file, void* v, size_t size)
{
    MemoryFile_ReallocIfNeeded(file, size);    
    memcpy(&file->data[file->pos], v, size);
    file->pos += size;

    if(file->pos > file->size)
    {
        file->size = file->pos;
    }

    return 0;
}

const char* MemoryFile_CurrentPosition(memoryFile_t* file)
{
    if(file->pos >= file->size)
    {
        return 0;
    }

    return &file->data[file->pos];
}

int MemoryFile_WriteUTF8NullTerminatedString(memoryFile_t* file, char* v, size_t size)
{
    size_t total_size = size + 1;

    if(!MemoryFile_ReallocIfNeeded(file, total_size))
    {
        return -1000;
    }

    memcpy(&file->data[file->pos], v, size);
    file->pos += size;
    file->data[file->pos++] = '\0';

    if(file->pos > file->size)
    {
        file->size = file->pos;
    }

    return MEMORYFILE_OK;
}

int MemoryFile_WriteUInt8(memoryFile_t* file, uint8_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteUInt16(memoryFile_t* file, uint16_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteUInt32(memoryFile_t* file, uint32_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteUInt64(memoryFile_t* file, uint64_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteInt8(memoryFile_t* file, int8_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteInt16(memoryFile_t* file, int16_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteInt32(memoryFile_t* file, int32_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteInt64(memoryFile_t* file, int64_t v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteFloat(memoryFile_t* file, float v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_WriteDouble(memoryFile_t* file, double v)
{
    return MemoryFile_Write(file, &v, sizeof(v));
}

int MemoryFile_Dump(const char* name, memoryFile_t* file)
{
    if(file == NULL)
        return MEMORYFILE_INVALID;
    if(file->data == NULL)
        return MEMORYFILE_NOTOPEN;

    FILE* fp;

    if((fp = fopen(name, "wb")) == NULL)
    {
        return MEMORYFILE_CANTCREATEDISKFILE;
    }

    fwrite(file->data, sizeof(char), file->size, fp);
    fclose(fp);
}

void MemoryFile_Close(memoryFile_t* file)
{
    if(file == NULL)
        return;
    if(file->data == NULL)
        return;

    free(file->data);

    file->pos = 0;
    file->size = 0;
    file->data = NULL;
}