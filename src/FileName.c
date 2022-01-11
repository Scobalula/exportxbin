#include "Common.h"
#include "Utility.h"
#include "FileName.h"

// TODO: Maybe pass in a buffer that the caller takes control of

int FileName_HasExtension(const char* full_path, const char* extension)
{
    if(full_path == NULL)
    {
        return 0;
    }

    for(size_t i = strlen(full_path) - 1; i >= 0; i--)
    {
        if(full_path[i] == '.')
        {
            if(strcasecmp(&full_path[i], extension) == 0)
            {
                return 1;
            }

            break;
        }
    }

    return 0;
}

char* FileName_GetFileNameWithoutExtension(const char* full_path)
{
    if(full_path == NULL)
    {
        return NULL;
    }

    size_t full_path_len = strlen(full_path);
    size_t start_of_file_name = 0;
    size_t extension_offset = full_path_len;

    for(long long i = full_path_len - 1; i >= 0; i--)
    {
        if(full_path[i] == '.')
        {
            extension_offset = i;
        }
        else if(full_path[i] == '\\' || full_path[i] == '/')
        {
            size_t start_of_file_name = i + 1;
            break;
        }
    }

    size_t result_len = extension_offset - start_of_file_name;
    
    if(result_len == 0)
    {
        return NULL;
    }

    char* result = malloc(result_len + 1);
    memcpy(result, &full_path[start_of_file_name], result_len);
    result[result_len] = '\0';

    return result;
}

char* FileName_GetFileName(const char* full_path)
{
    if(full_path == NULL)
    {
        return NULL;
    }

    size_t full_path_len = strlen(full_path);
    size_t start_of_file_name = 0;

    for(size_t i = full_path_len - 1; i >= 0; i--)
    {
        if(full_path[i] == '\\' || full_path[i] == '/')
        {
            start_of_file_name = i + 1;
            break;
        }
    }

    size_t result_len = full_path_len - start_of_file_name;

    if(result_len == 0)
    {
        return NULL;
    }

    char* result = malloc(result_len + 1);
    memcpy(result, &full_path[start_of_file_name], result_len);
    result[result_len] = '\0';

    return result;
}

char* FileName_GetExtension(const char* full_path)
{
    if(full_path == NULL)
    {
        return NULL;
    }

    size_t full_path_len = strlen(full_path);
    size_t start_of_extension = 0;

    for(size_t i = full_path_len - 1; i >= 0; i--)
    {
        if(full_path[i] == '.')
        {
            start_of_extension = i;
            break;
        }
    }

    size_t result_len = full_path_len - start_of_extension;

    if(result_len == 0)
    {
        return NULL;
    }

    char* result = malloc(result_len + 1);
    memcpy(result, &full_path[start_of_extension], result_len);
    result[result_len] = '\0';

    return result;
}

char* FileName_ChangeExtension(const char* full_path, const char* extension)
{
    if(full_path == NULL)
    {
        return NULL;
    }

    size_t new_ext_len = strlen(extension);
    size_t full_path_len = strlen(full_path);
    size_t start_of_extension = 0;

    for(size_t i = full_path_len - 1; i >= 0; i--)
    {
        if(full_path[i] == '.')
        {
            start_of_extension = i;
            break;
        }
    }

    size_t result_len = start_of_extension + new_ext_len;

    if(result_len == 0)
    {
        return NULL;
    }

    char* result = malloc(result_len + 1);
    memcpy(result, full_path, start_of_extension);
    memcpy(&result[start_of_extension], extension, new_ext_len);
    result[result_len] = '\0';

    return result; 
}

char* FileName_GetDirectory(const char* full_path)
{
    if(full_path == NULL)
    {
        return NULL;
    }

    size_t full_path_len = strlen(full_path);
    size_t start_of_file_name = 0;

    for(size_t i = full_path_len - 1; i >= 0; i--)
    {
        if(full_path[i] == '\\')
        {
            start_of_file_name = i;
            break;
        }
    }

    size_t result_len = start_of_file_name;

    if(result_len == 0)
    {
        return NULL;
    }

    char* result = malloc(result_len + 1);
    memcpy(result, full_path, result_len);
    result[result_len] = '\0';

    return result;
}

void FileName_FreeResult(char* file_name_result)
{
    if(file_name_result == NULL)
    {
        return;
    }

    free(file_name_result);
}