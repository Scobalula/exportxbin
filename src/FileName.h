#ifndef FILENAME_INCLUDED
#define FILENAME_INCLUDED

int FileName_HasExtension(const char* full_path, const char* extension);

char* FileName_GetFileNameWithoutExtension(const char* full_path);

char* FileName_GetFileName(const char* full_path);

char* FileName_GetExtension(const char* full_path);

char* FileName_ChangeExtension(const char* full_path, const char* extension);

char* FileName_GetDirectory(const char* full_path);

void FileName_FreeResult(char* file_name_result);

#endif