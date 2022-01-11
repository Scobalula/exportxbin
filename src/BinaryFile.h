#ifndef BINARYFILE_INCLUDED
#define BINARYFILE_INCLUDED
#include "Common.h"
#include "Utility.h"
#include "Token.h"
#include "MemoryFile.h"

int BinaryFile_Open(const char* file_name, memoryFile_t* file, int flags);

int BinaryFile_Save(const char* file_name, memoryFile_t* file, int flags);

int BinaryReader_ReadToken(memoryFile_t* file, tokenData_t* data);

int BinaryFile_WriteToken(memoryFile_t* file, tokenData_t* data);
#endif