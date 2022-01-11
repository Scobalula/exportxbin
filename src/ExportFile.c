#include "ExportFile.h"

int ExportFile_SkipWhitespace(memoryFile_t *file, int consider_line_end_whitespace)
{
    while (file->pos < file->size)
    {
        if (file->data[file->pos] == '\n' || file->data[file->pos] == '\r')
        {
            if(!consider_line_end_whitespace)
            {
                file->pos++;

                if(file->data[file->pos] == '\r')
                    file->pos++;

                return -1;
            }
        }
        else if (file->data[file->pos] != ' ' && file->data[file->pos] != '\t')
        {
            return 1;
        }

        file->pos++;
    }

    return 0;
}

int ExportFile_IsNextAStringLiteral(memoryFile_t *file)
{
    if (file->pos >= file->size)
    {
        return -1;
    }

    return file->data[file->pos] == '"';
}

int ExportFile_SkipValue(memoryFile_t *file)
{
    if (file->pos >= file->size)
    {
        return 0;
    }

    if (file->data[file->pos] == '\n' || file->data[file->pos] == '\r')
    {
        return -1;
    }

    if(ExportFile_SkipWhitespace(file, 0) != 1)
    {
        return 0;
    }

    while (file->pos < file->size)
    {
        if (file->data[file->pos] == '"')
        {
            file->pos++;

            while (file->pos < file->size)
            {
                char val = file->data[file->pos++];

                if (val == '"')
                {
                    return 1;
                }

                if (val == '\n')
                {
                    return -2;
                }
            }
        }
        else
        {
            while (file->pos < file->size)
            {
                char val = file->data[file->pos++];

                if (val == ' ' || val == '\n')
                {
                    return 1;
                }
            }
        }
    }
}

int ExportFile_ParseComment(memoryFile_t *file, tokenData_t *data)
{
    if(!ExportFile_SkipWhitespace(file, 0))
    {
        return 1;
    }

    size_t start = file->pos;
    size_t end = file->pos;
    char curr = 0;

    while (file->pos < file->size)
    {
        if (MemoryFile_ReadChar(file, &curr) != MEMORYFILE_OK)
        {
            printf("| ERROR: Failed to read comment character in %s.\n", file->name);
            return 0;
        }

        if (curr == '\n' || curr == '\r')
        {
            break;
        }

        end++;
    }

    size_t string_size = end - start;

    data->stringVal = malloc(string_size + 1);
    memset(data->stringVal, 0, string_size + 1);
    memcpy(data->stringVal, &file->data[start], string_size);

    return 1;
}

int ExportFile_ReadValue(memoryFile_t* file, char** dst, size_t* dst_len)
{
    if(ExportFile_SkipWhitespace(file, 0) != 1)
    {
        printf("| ERROR: Error reading token in %s.\n", file->name);
        return 0;
    }

    if (file->pos >= file->size)
    {
        printf("| ERROR: Attempted to read beyond the end of %s while reading token.\n", file->name);
        return 0;
    }

    if (file->data[file->pos] == '\n')
    {
        printf("| ERROR: Hit end of line while reading token in %s.\n", file->name);
        return 0;
    }

    int type = 1;
    size_t start = file->pos;
    size_t end = file->pos;

    if(file->data[file->pos] == '/' && file->data[file->pos + 1] == '/')
    {
        end = start + 2;
        file->pos += 2;
    }
    else if (file->data[file->pos] == '"')
    {
        type = 2;
        file->pos++;
        start = file->pos;

        while (file->pos < file->size)
        {
            char val = file->data[file->pos++];

            if (val == '"')
            {
                end = file->pos - 1;
                break;
            }
            if (val == '\n')
            {
                printf("| ERROR: Hit end of line in %s while reading string literal.\n", file->name);
                return 0;
            }
        }
    }
    else
    {
        while (file->pos < file->size)
        {
            if(file->data[file->pos] == '\n' ||
               file->data[file->pos] == '\r' ||
               file->data[file->pos] == '\t' ||
               file->data[file->pos] == ' ')
            {
                break;
            }
            
            file->pos++;
        }

        end = file->pos;
    }

    if(dst_len != NULL)
        *dst_len = end - start;
    if(dst != NULL)
        *dst = &file->data[start];
    return type;
}

int ExportFile_ReadStringLiteral(memoryFile_t *file, char** v)
{
    if(!ExportFile_SkipWhitespace(file, 0))
    {
        return 0;
    }

    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 2)
    {
        printf("| ERROR: Failed to read string literal from %s.\n", file->name);
        return 0;
    }

    *v = malloc(len + 1);
    memset(*v, 0, len + 1);
    memcpy(*v, str, len);

    return 1;
}

int ExportFile_ReadUChar(memoryFile_t* file, unsigned char* v)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read byte value from %s.\n", file->name);
        return 0;
    }

    char num[256];
    memset(num, 0, sizeof(num));
    errno_t e = strncpy_s(num, 256, str, len);
    int result = atoi(num);

    if(result > 0xFF)
    {
        printf("| ERROR: Byte value out of range in %s.\n", file->name);
        return 0;
    }

    *v = (unsigned char)result;
    return 1;
}

int ExportFile_ReadShort(memoryFile_t* file, short* v)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read short value from %s.\n", file->name);
        return 0;
    }

    char num[256];
    memset(num, 0, sizeof(num));
    errno_t e = strncpy_s(num, 256, str, len);
    int result = atoi(num);

    if(result < -32767 || result > 32768)
    {
        printf("| ERROR: Failed to read short value from %s.\n", file->name);
        return 0;
    }

    *v = (short)result;
    return 1;
}

int ExportFile_ReadUShort(memoryFile_t* file, unsigned short* v)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read short value from %s.\n", file->name);
        return 0;
    }

    char num[256];
    memset(num, 0, sizeof(num));
    errno_t e = strncpy_s(num, 256, str, len);
    unsigned long long result = atoll(num);
    

    if(result > 0xFFFF)
    {
        printf("| ERROR: Unsigned short value out of range in %s.\n", file->name);
        return 0;
    }

    *v = (unsigned short)result;
    return 1;
}

int ExportFile_ReadInt(memoryFile_t* file, int* v)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read int value from %s.\n", file->name);
        return 0;
    }

    char num[256];
    memset(num, 0, sizeof(num));
    errno_t e = strncpy_s(num, 256, str, len);
    long long result = atoll(num);

    if(result > 0xFFFFFFFF)
    {
        printf("| ERROR: Int value out of range in %s.\n", file->name);
        return 0;
    }

    *v = (int)result;
    return 1;
}

int ExportFile_ReadUInt(memoryFile_t* file, unsigned int* v)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read int value from %s.\n", file->name);
        return 0;
    }

    char num[256];
    memset(num, 0, sizeof(num));
    errno_t e = strncpy_s(num, 256, str, len);
    unsigned long long result = atoll(num);

    if(result > 0xFFFFFFFF)
    {
        printf("| ERROR: Int value out of range in %s.\n", file->name);
        return 0;
    }

    *v = (unsigned int)result;
    return 1;
}

int ExportFile_ReadFloat(memoryFile_t* file, float* v)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read float value from %s.\n", file->name);
        return 0;
    }

    char num[256];
    memset(num, 0, sizeof(num));
    errno_t e = strncpy_s(num, 256, str, len);
    float result = atof(num);
    *v = (float)result;
    return 1;    
}

int ExportFile_ReadUNormalizedFloat(memoryFile_t* file, float* v)
{
    if(!ExportFile_ReadFloat(file, v))
    {
        return 0;
    }

    if(*v < 0 || *v > 1.0)
    {
        printf("| ERROR: UNorm float value out of range in %s.\n", file->name);
        return 0;
    }

    return 1;    
}

int ExportFile_ReadNormalizedFloat(memoryFile_t* file, float* v)
{
    if(!ExportFile_ReadFloat(file, v))
    {
        return 0;
    }

    if(*v < -1.0 || *v > 1.0)
    {
        printf("| ERROR: UNorm float value out of range in %s.\n", file->name);
        return 0;
    }

    return 1;    
}

int ExportFile_ResolvePotentialDataCount(memoryFile_t* file)
{
    if(file->pos >= file->size)
    {
        return 0;
    }

    int count = 0;

    while (file->pos < file->size)
    {
        int r_val = ExportFile_SkipValue(file);

        if(r_val == 0)
        {
            printf("| ERROR: Failed to read from file %s.\n", file->name);
            return -1;
        }
        else if(r_val == -1)
        {
            break;
        }
        else if(r_val == -2)
        {
            printf("|ERROR: Line end in string literal %s.\n", file->name);
            return -1;         
        }
        else
        {
            count++;  
        }

        if(ExportFile_SkipWhitespace(file, 0) != 1)
        {
            break;
        }

        // If the next item is potentially a token, then we can now quit
        if(Token_TryFindFromName(&file->data[file->pos], -1, NULL))
        {
            break;
        }
    }
    
    return count;
}

void ExportFile_TryConvertToWiderType(tokenData_t* data)
{
    // TODO: Implement a check, but for now
    // we can just force them to a wider type
    // regardless of if they contain wider values
    if(!strcmp(data->token.name, "VERT"))
    {
        Token_TryFindFromName("VERT32", -1, data);
    }
    else if(!strcmp(data->token.name, "NUMVERTS"))
    {
        Token_TryFindFromName("NUMVERTS32", -1, data);
    }
    else if(!strcmp(data->token.name, "TRI"))
    {
        Token_TryFindFromName("TRI16", -1, data);
    }
}

int ExportFile_ReadTokenName(memoryFile_t* file, tokenData_t* data)
{
    char* str = NULL;
    size_t len = 0;

    if(ExportFile_ReadValue(file, &str, &len) != 1)
    {
        printf("| ERROR: Failed to read token name from %s.\n", file->name);
        return 0;
    }

    return Token_TryFindFromName(str, len, data);
}

int ExportFile_ResolveToken(memoryFile_t *file, tokenData_t *data, size_t* potential_data_count)
{
    // Since Treyarch decided to give multiple unique 
    // "tokens" the exact same name, we must try and 
    // resolve it using the number of elements we have
    // this is fairly close to how export2bin handles it
    // it has a loop over the data and does multiple checks
    // jumping to the next item to check again
    if (!ExportFile_ReadTokenName(file, data))
    {
        printf("| ERROR: Failed to find token in %s.\n", file->name);
        return 0;
    }

    ExportFile_TryConvertToWiderType(data);

    size_t start_position = file->pos;

    int count = ExportFile_ResolvePotentialDataCount(file);

    if(count < 0)
    {
        printf("| ERROR: Failed to resolve token data count in %s.\n", file->name);
        return 0;
    }

    while (1)
    {
        int match = 1;

        switch (data->token.data_type)
        {
        case 0:
            break;
        case 1:
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            match = count == 1;
            break;
        case 6:
            match = count == 4;
            break;
        case 7:
            match = count == 3;
            break;
        case 8:
            match = count == 1;
            break;
        case 9:
            match = count == 2;
            break;
        case 10:
            match = count == 3;
            break;
        case 11:
            match = count == 4;
            break;
        case 12:
            match = count == 2;
            break;
        case 13:
            match = count > 1;
            break;
        case 14:
            match = count == 2;
            break;
        case 15:
            match = count == 4;
            break;
        case 17:
            match = count == 3;
            break;
        case 18:
            match = count >= 2;
            break;
        case 19:
            match = count >= 2;
            break;
        default:
            break;
        }

        if (match)
        {
            break;
        }

        if (!Token_TryGetNextToken(data))
        {
            // We had a valid token, but invalid data
            printf("| ERROR: Invalid token data %s in %s.\n", data->token.name, file->name);
            return -1;
        }
    }

    // Make sure we're back at the start of the token
    file->pos = start_position;
    *potential_data_count = (size_t)count;
    return 1;
}

int ExportFile_ReadToken(memoryFile_t *file, tokenData_t *data)
{
    ExportFile_SkipWhitespace(file, 1);

    if(file->pos >= file->size)
    {
        return -1;
    }

    size_t potential_data_count;
    int result = ExportFile_ResolveToken(file, data, &potential_data_count);

    if (result != 1)
    {
        printf("| ERROR: Failed to resolve token in %s.\n", file->name);
        return 0;
    }

    switch (data->token.data_type)
    {
    case 0:
        result = ExportFile_ParseComment(file, data);
        break;
    case 1:
        break;
    case 2:
        result = ExportFile_ReadShort(file, &data->shortVal);
        break;
    case 3:
        result = ExportFile_ReadUShort(file, &data->shortVal);
        break;
    case 4:
        result = ExportFile_ReadInt(file, &data->intVal);
        break;
    case 5:
        result = ExportFile_ReadUInt(file, &data->intVal);
        break;
    case 6:
        result = ExportFile_ReadUNormalizedFloat(file, &data->vec4Val[0]);
        if(result)
            result = ExportFile_ReadUNormalizedFloat(file, &data->vec4Val[1]);
        if(result)
            result = ExportFile_ReadUNormalizedFloat(file, &data->vec4Val[2]);
        if(result)
            result = ExportFile_ReadUNormalizedFloat(file, &data->vec4Val[3]);
        break;
    case 7:
        result = ExportFile_ReadNormalizedFloat(file, &data->vec3Val[0]);
        if(result)
            result = ExportFile_ReadNormalizedFloat(file, &data->vec3Val[1]);
        if(result)
            result = ExportFile_ReadNormalizedFloat(file, &data->vec3Val[2]);
        break;
    case 8:
        result = ExportFile_ReadFloat(file, &data->floatVal);
        break;
    case 9:
        result = ExportFile_ReadFloat(file, &data->vec2Val[0]);
        if(result)
            result = ExportFile_ReadFloat(file, &data->vec2Val[1]);  
        break;
    case 10:
        result = ExportFile_ReadFloat(file, &data->vec3Val[0]);
        if(result)
            result = ExportFile_ReadFloat(file, &data->vec3Val[1]);
        if(result)
            result = ExportFile_ReadFloat(file, &data->vec3Val[2]); 
        break;
    case 11:
        result = ExportFile_ReadFloat(file, &data->vec4Val[0]);
        if(result)
            result = ExportFile_ReadFloat(file, &data->vec4Val[1]);
        if(result)
            result = ExportFile_ReadFloat(file, &data->vec4Val[2]);
        if(result)
            result = ExportFile_ReadFloat(file, &data->vec4Val[3]);
        break;
    case 12:
        result = ExportFile_ReadUShort(file, &data->bone_weight_val.index);
        if(result)
            result = ExportFile_ReadUNormalizedFloat(file, &data->bone_weight_val.weight);
        break;
    case 13:
        result = ExportFile_ReadUShort(file, &data->uvSet.elem_count);
        // Export2Bin imposes 16 Max (not used in current tools anyway)
        if(result && data->uvSet.elem_count > 16)
        {
            printf("| ERROR: Exceeded UV set count in %s.\n", file->name);
            result = 0;
        }
        // Error correction
        if(data->uvSet.elem_count == 0 && potential_data_count >= 3)
        {
            data->uvSet.elem_count = ((uint16_t)potential_data_count - 1) / 2;
        }
        for (size_t uv = 0; uv < data->uvSet.elem_count && result; uv++)
        {
            result = ExportFile_ReadFloat(file, &data->uvSet.sets[uv].x);
            if(result)
                result = ExportFile_ReadFloat(file, &data->uvSet.sets[uv].y);
        }
        break;
    case 14:
        result = ExportFile_ReadUShort(file, &data->part_val.shortVal);
        if(result)
            result = ExportFile_ReadStringLiteral(file, &data->part_val.stringVal);
        break;
    case 15:
        result = ExportFile_ReadUShort(file, &data->material_val.shortVal);
        if(result)
            result = ExportFile_ReadStringLiteral(file, &data->material_val.stringVal0);
        if(result)
            result = ExportFile_ReadStringLiteral(file, &data->material_val.stringVal1);
        if(result)
            result = ExportFile_ReadStringLiteral(file, &data->material_val.stringVal2);
        break;
    case 17:
        result = ExportFile_ReadInt(file, &data->model_bone_val.index);
        if(result)
            result = ExportFile_ReadInt(file, &data->model_bone_val.parent_index);
        if(result)
            result = ExportFile_ReadStringLiteral(file, &data->model_bone_val.name);
        break;
    case 18:
        result = ExportFile_ReadUChar(file, &data->tri_val.material_index);
        if(result)
            result = ExportFile_ReadUChar(file, &data->tri_val.object_index);
        if(result)
            result = ExportFile_ReadUChar(file, &data->tri_val.unused_val0);
        if(result)
            result = ExportFile_ReadUChar(file, &data->tri_val.unused_val1);
        break;
    case 19:
        result = ExportFile_ReadUShort(file, &data->tri16_val.material_index);
        if(result)
            result = ExportFile_ReadUShort(file, &data->tri16_val.object_index);
        if(result)
            result = ExportFile_ReadUShort(file, &data->tri16_val.unused_val0);
        if(result)
            result = ExportFile_ReadUShort(file, &data->tri16_val.unused_val1);
    default:
        break;
    }

    return result;
}

int ExportFile_WriteToken(FILE *fp, tokenData_t *data)
{
    int result = 0;

    switch (data->token.data_type)
    {
    case 0:
        result = fprintf(fp, "%s %s\n", data->token.name, data->stringVal);
        break;
    case 1:
        result = fprintf(fp, "%s\n", data->token.name);
        break;
    case 2:
        result = fprintf(fp, "%s %i\n", data->token.name, data->shortVal);
        break;
    case 3:
        result = fprintf(fp, "%s %i\n", data->token.name, data->ushortVal);
        break;
    case 4:
        result = fprintf(fp, "%s %i\n", data->token.name, data->intVal);
        break;
    case 5:
        result = fprintf(fp, "%s %i\n", data->token.name, data->uintVal);
        break;
    case 6:
        result = fprintf(fp, "%s %f %f %f %f\n", data->token.name,
                         data->vec4Val[0],
                         data->vec4Val[1],
                         data->vec4Val[2],
                         data->vec4Val[3]);
        break;
    case 7:
        result = fprintf(fp, "%s %f %f %f\n", data->token.name,
                         data->vec3Val[0],
                         data->vec3Val[1],
                         data->vec3Val[2]);
        break;
    case 8:
        result = fprintf(fp, "%s %f\n", data->token.name,
                         data->floatVal);
        break;
    case 9:
        result = fprintf(fp, "%s %f %f\n", data->token.name,
                         data->vec2Val[0],
                         data->vec2Val[1]);
        break;
    case 10:
        result = fprintf(fp, "%s %f %f %f\n", data->token.name,
                         data->vec3Val[0],
                         data->vec3Val[1],
                         data->vec3Val[2]);
        break;
    case 11:
        result = fprintf(fp, "%s %f %f %f %f\n", data->token.name,
                         data->vec4Val[0],
                         data->vec4Val[1],
                         data->vec4Val[2],
                         data->vec4Val[3]);
        break;
    case 12:
        result = fprintf(fp, "%s %i %f\n", data->token.name,
                         data->bone_weight_val.index,
                         data->bone_weight_val.weight);
        break;
    case 13:
        result = fprintf(fp, "%s %i", data->token.name, data->uvSet.elem_count);
        for (size_t i = 0; i < data->uvSet.elem_count; i++)
        {
            result = fprintf(fp, " %f %f",
                             data->uvSet.sets[i].x,
                             data->uvSet.sets[i].y);
        }
        result = fprintf(fp, "\n");
        break;
    case 14:
        result = fprintf(fp, "%s %i \"%s\"\n", data->token.name,
                         data->part_val.shortVal,
                         data->part_val.stringVal);
        break;
    case 15:
        result = fprintf(fp, "%s %i \"%s\" \"%s\" \"%s\"\n", data->token.name,
                         data->material_val.shortVal,
                         data->material_val.stringVal0,
                         data->material_val.stringVal1,
                         data->material_val.stringVal2);
        break;
    case 17:
        result = fprintf(fp, "%s %i %i \"%s\"\n", data->token.name,
                         data->model_bone_val.index,
                         data->model_bone_val.parent_index,
                         data->model_bone_val.name);
        break;
    case 18:
        result = fprintf(fp, "%s %i %i 0 0\n", data->token.name,
                         data->tri_val.material_index,
                         data->tri_val.object_index);
        break;
    case 19:
        result = fprintf(fp, "%s %i %i 0 0\n", data->token.name,
                         data->tri16_val.material_index,
                         data->tri16_val.object_index);
        break;
    default:
        return -12;
    }

    return result;
}