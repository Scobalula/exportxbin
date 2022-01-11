#ifndef MEMORYFILE_INCLUDED
#define MEMORYFILE_INCLUDED

#define MEMORYFILE_DEFAULT_STR_SIZE      32

#define MEMORYFILE_OK                    0
#define MEMORYFILE_INVALID              -1
#define MEMORYFILE_NOTOPEN              -2
#define MEMORYFILE_PASTEND              -4
#define MEMORYFILE_CANTOPENDISKFILE     -5
#define MEMORYFILE_CANTCREATEDISKFILE   -6

typedef struct
{
    const char* name;
    unsigned long long size;
    unsigned long long capacity;
    unsigned long long pos;
    char* data;
} memoryFile_t;

int MemoryFile_InitBlank(memoryFile_t* file, size_t initial_buffer_size);

int MemoryFile_ConsumeFromFile(const char* name, memoryFile_t* file);

size_t MemoryFile_Align(memoryFile_t* file, size_t alignment);

int MemoryFile_Read(memoryFile_t* file, void* v, size_t size_of_data);

int MemoryFile_ReadChar(memoryFile_t* file, char* v);

int MemoryFile_ReadInt8(memoryFile_t* file, signed char* v);

int MemoryFile_ReadInt32(memoryFile_t* file, int* v);

int MemoryFile_ReadInt16(memoryFile_t* file, short* v);

int MemoryFile_ReadInt64(memoryFile_t* file, long long* v);

int MemoryFile_ReadUInt8(memoryFile_t* file, unsigned char* v);

int MemoryFile_ReadUInt16(memoryFile_t* file, unsigned short* v);

int MemoryFile_ReadUInt32(memoryFile_t* file, unsigned int* v);

int MemoryFile_ReadUInt64(memoryFile_t* file, unsigned long long* v);

int MemoryFile_ReadFloat(memoryFile_t* file, float* v);

int MemoryFile_ReadDouble(memoryFile_t* file, double* v);

int MemoryFile_ReadUTF8NullTerminatedString(memoryFile_t* file, char** v, size_t* size_result);

const char* MemoryFile_CurrentPosition(memoryFile_t* file);

int MemoryFile_Dump(const char* name, memoryFile_t* file);

int MemoryFile_Write(memoryFile_t* file, void* v, size_t size);

int MemoryFile_WriteUInt8(memoryFile_t* file, uint8_t v);

int MemoryFile_WriteUInt16(memoryFile_t* file, uint16_t v);

int MemoryFile_WriteUInt32(memoryFile_t* file, uint32_t v);

int MemoryFile_WriteUInt64(memoryFile_t* file, uint64_t v);

int MemoryFile_WriteInt8(memoryFile_t* file, int8_t v);

int MemoryFile_WriteInt16(memoryFile_t* file, int16_t v);

int MemoryFile_WriteInt32(memoryFile_t* file, int32_t v);

int MemoryFile_WriteInt64(memoryFile_t* file, int64_t v);

int MemoryFile_WriteFloat(memoryFile_t* file, float v);

int MemoryFile_WriteDouble(memoryFile_t* file, double v);

int MemoryFile_WriteUTF8NullTerminatedString(memoryFile_t* file, char* v, size_t size);

void MemoryFile_Close(memoryFile_t* file);

#endif