#ifndef FILESYSTEM_INCLUDED
#define FILESYSTEM_INCLUDED
#include "Globals.h"
#include "Utility.h"
#include "Common.h"

int FileSystem_IsDirectory(const char* dir);

int FileSystem_IsFile(const char* file_path);

int FileSystem_EnumerateFiles(const char* dir, const int sub_dirs, const char* search, void(*callback)(const char*, const char*));

#endif