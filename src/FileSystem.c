#include <Windows.h>
#include "FileSystem.h"

int FileSystem_IsDirectory(const char* dir)
{
    int attrib = GetFileAttributes(dir);

    if (attrib == INVALID_FILE_ATTRIBUTES)
        return 0;
    if ((attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
        return 0;

    return 1;
}

int FileSystem_IsFile(const char* file_path)
{
    int attrib = GetFileAttributes(file_path);

    if (attrib == INVALID_FILE_ATTRIBUTES)
        return 0;
    if ((attrib & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return 0;

    return 1;
}

int FileSystem_EnumerateFiles(const char* dir, const int sub_dirs, const char* search, void(*callback)(const char*, const char*))
{
    errno_t r;
    char full_search[MAX_PATH];
    char subdir_name[MAX_PATH];

    int p_result = sprintf_s(full_search, sizeof(full_search), "%s\\%s", dir, search);

    WIN32_FIND_DATAA find;
    HANDLE h = FindFirstFileA(full_search, &find);

    if(h == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    do
    {
        if(strcmp(find.cFileName, ".") && strcmp(find.cFileName, ".."))
        {
            if (sub_dirs && (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                int p_result = sprintf_s(subdir_name, sizeof(subdir_name), "%s\\%s", dir, find.cFileName);
                FileSystem_EnumerateFiles(subdir_name, sub_dirs, search, callback);
            }
            else
            {
                callback(dir, find.cFileName);
            }
        }
    } while(FindNextFileA(h, &find));

    return 1;
}