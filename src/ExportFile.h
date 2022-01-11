#ifndef EXPORTFILE_INCLUDED
#define EXPORTFILE_INCLUDED
#include "Utility.h"
#include "Common.h"
#include "MemoryFile.h"
#include "Token.h"

int ExportFile_ReadToken(memoryFile_t* file, tokenData_t* data);

int ExportFile_WriteToken(FILE* fp, tokenData_t* data);

#endif