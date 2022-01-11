#include "lz4hc.h"
#include "BinaryFile.h"

int BinaryFile_Open(const char* file_name, memoryFile_t* file, int flags)
{
    memoryFile_t temp;

    if(MemoryFile_ConsumeFromFile(file_name, &temp) != MEMORYFILE_OK)
    {
        return -1;
    }

    char magic[5];
    uint32_t raw_size = 0;

    if(MemoryFile_Read(&temp, &magic, sizeof(magic)) != MEMORYFILE_OK)
    {
        return -2;
    }
    if(MemoryFile_Read(&temp, (void*)&raw_size, sizeof(raw_size)) != MEMORYFILE_OK)
    {
        return -2;
    }

    MemoryFile_InitBlank(file, raw_size);
    file->size = raw_size;
    size_t size_of_compressed = temp.size - 9;
    int result = LZ4_decompress_safe(&temp.data[temp.pos], file->data, size_of_compressed, raw_size);
    MemoryFile_Close(&temp);

    if(result != raw_size)
    {
        return result;
    }

    return 0;
}

int BinaryFile_Save(const char* file_name, memoryFile_t* file, int flags)
{
    int result;
    FILE* fp;

    if((fp = fopen(file_name, "wb")) == NULL)
    {
        return -1;
    }

    if((result = fwrite("*LZ4*", 1, 5, fp)) != 5)
    {
        return -2;
    }

    int decompressed_size = (int)file->size;
    int estimated_size = LZ4_compressBound(decompressed_size);
    char* decompressed_buffer = malloc(estimated_size);

    result = LZ4_compress_default(file->data, decompressed_buffer, decompressed_size, estimated_size);

    if(result > 0)
    {
        fwrite(&decompressed_size, sizeof(decompressed_size), 1, fp);
        fwrite(decompressed_buffer, result, 1, fp);
    }

    fclose(fp);

    free(decompressed_buffer);
    return result;
}

int BinaryReader_ReadToken(memoryFile_t* file, tokenData_t* data)
{
    MemoryFile_Align(file, 4);

    uint16_t crc = 0;
    int result = MemoryFile_ReadUInt16(file, &crc);

    if(result != MEMORYFILE_OK)
        return 0;

    if(!Token_TryFindFromCRC(crc, data))
        return -1;

    switch (data->token.data_type)
    {
    case 0:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadUTF8NullTerminatedString(file, &data->stringVal, NULL) != MEMORYFILE_OK)
            return -2;
        break;      
    case 1:
        break;
    case 2:
        if(MemoryFile_ReadInt16(file, &data->shortVal) != MEMORYFILE_OK)
            return -2;
        break;
    case 3:
        if(MemoryFile_ReadUInt16(file, &data->ushortVal) != MEMORYFILE_OK)
            return -2;
        break;
    case 4:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadInt32(file, &data->intVal) != MEMORYFILE_OK)
            return -2;
        break;
    case 5:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadUInt32(file, &data->uintVal) != MEMORYFILE_OK)
            return -2;
        break;
    case 6:
        MemoryFile_Align(file, 4);
        for (size_t i = 0; i < 4; i++)
        {
            unsigned char v = 0;
            if(MemoryFile_ReadUInt8(file, &v) != MEMORYFILE_OK)
                return -2;
            data->vec4Val[i] = v / 255.0f;
        }
        break;
    case 7:
        MemoryFile_Align(file, 2);
        for (size_t i = 0; i < 3; i++)
        {
            short v = 0;
            if(MemoryFile_ReadInt16(file, &v) != MEMORYFILE_OK)
                return -2;
            data->vec3Val[i] = v / 32767.0f;
        }
        break;
    case 8:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadFloat(file, &data->floatVal) != MEMORYFILE_OK)
            return -2;
        break;
    case 9:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadFloat(file, &data->vec2Val[0]) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadFloat(file, &data->vec2Val[1]) != MEMORYFILE_OK)
            return -2;
        break;
    case 10:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadFloat(file, &data->vec3Val[0]) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadFloat(file, &data->vec3Val[1]) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadFloat(file, &data->vec3Val[2]) != MEMORYFILE_OK)
            return -2;
        break;
    case 11:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadFloat(file, &data->vec4Val[0]) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadFloat(file, &data->vec4Val[1]) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadFloat(file, &data->vec4Val[2]) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadFloat(file, &data->vec4Val[3]) != MEMORYFILE_OK)
            return -2;
        break;
    case 12:
        if(MemoryFile_ReadUInt16(file, &data->bone_weight_val.index) != MEMORYFILE_OK)
            return -2;
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadFloat(file, &data->bone_weight_val.weight) != MEMORYFILE_OK)
            return -2;
        break;
    case 13:
        if(MemoryFile_ReadUInt16(file, &data->uvSet.elem_count) != MEMORYFILE_OK)
            return -2;
        // Export2Bin imposes 16 Max
        if(data->uvSet.elem_count > 16)
            return -2;
        MemoryFile_Align(file, 4);
        for (unsigned short i = 0; i < data->uvSet.elem_count; i++)
        {
            if(MemoryFile_ReadFloat(file, &data->uvSet.sets[i].x) != MEMORYFILE_OK)
                return -2;
            if(MemoryFile_ReadFloat(file, &data->uvSet.sets[i].y) != MEMORYFILE_OK)
                return -2;
        }
        break;
    case 14:
        if(MemoryFile_ReadUInt16(file, &data->part_val.shortVal) != MEMORYFILE_OK)
            return -2;
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadUTF8NullTerminatedString(file, &data->part_val.stringVal, NULL) != MEMORYFILE_OK)
            return -2;
        break;
    case 15:
        if(MemoryFile_ReadUInt16(file, &data->material_val.shortVal) != MEMORYFILE_OK)
            return -2;
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadUTF8NullTerminatedString(file, &data->material_val.stringVal0, NULL) != MEMORYFILE_OK)
            return -2;
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadUTF8NullTerminatedString(file, &data->material_val.stringVal1, NULL) != MEMORYFILE_OK)
            return -2;
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadUTF8NullTerminatedString(file, &data->material_val.stringVal2, NULL) != MEMORYFILE_OK)
            return -2;
        break;
    case 17:
        MemoryFile_Align(file, 4);
        if(MemoryFile_ReadInt32(file, &data->model_bone_val.index) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadInt32(file, &data->model_bone_val.parent_index) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadUTF8NullTerminatedString(file, &data->model_bone_val.name, NULL) != MEMORYFILE_OK)
            return -2;
        break;
    case 18:
        if(MemoryFile_ReadUInt8(file, &data->tri_val.material_index) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadUInt8(file, &data->tri_val.object_index) != MEMORYFILE_OK)
            return -2;
        break;
    case 19:
        if(MemoryFile_ReadUInt16(file, &data->tri16_val.material_index) != MEMORYFILE_OK)
            return -2;
        if(MemoryFile_ReadUInt16(file, &data->tri16_val.object_index) != MEMORYFILE_OK)
            return -2;
        break;
    default:
        return -2;
    }

    return 1;
}

int BinaryFile_WriteToken(memoryFile_t* file, tokenData_t* data)
{
    MemoryFile_Align(file, 4);
    int result = MemoryFile_WriteUInt16(file, data->token.hash);

    if(result != MEMORYFILE_OK)
        return result;

    switch (data->token.data_type)
    {
    case 0:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteUTF8NullTerminatedString(file, data->stringVal, strlen(data->stringVal));
        break;      
    case 1:
        break;
    case 2:
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteInt16(file, data->shortVal);
        break;
    case 3:
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteUInt16(file, data->ushortVal);
        break;
    case 4:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteInt32(file, data->intVal);
        break;
    case 5:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteUInt32(file, data->uintVal);
        break;
    case 6:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteUInt8(file, (uint8_t)(data->vec4Val[1] * 255.0f));
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUInt8(file, (uint8_t)(data->vec4Val[1] * 255.0f));
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUInt8(file, (uint8_t)(data->vec4Val[2] * 255.0f));
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUInt8(file, (uint8_t)(data->vec4Val[3] * 255.0f));
        break;
    case 7:
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteInt16(file, (int16_t)(data->vec3Val[0] * 32767.0f));
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteInt16(file, (int16_t)(data->vec3Val[1] * 32767.0f));
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteInt16(file, (int16_t)(data->vec3Val[2] * 32767.0f));
        break;
    case 8:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteFloat(file, data->floatVal);
        break;
    case 9:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteFloat(file, data->vec2Val[0]);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->vec2Val[1]);
        break;
    case 10:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteFloat(file, data->vec2Val[0]);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->vec3Val[1]);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->vec3Val[2]);
        break;
    case 11:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteFloat(file, data->vec4Val[0]);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->vec4Val[1]);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->vec4Val[2]);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->vec4Val[3]);
        break;
    case 12:
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteUInt16(file, data->bone_weight_val.index);
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteFloat(file, data->bone_weight_val.weight);
        break;
    case 13:
        // Export2Bin imposes 16 Max
        if(data->uvSet.elem_count > 16)
            return -2;
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteUInt16(file, data->uvSet.elem_count);
        for (uint16_t i = 0; i < data->uvSet.elem_count && result == MEMORYFILE_OK; i++)
        {
            MemoryFile_Align(file, 4);
            result = MemoryFile_WriteFloat(file, data->uvSet.sets[i].x);
            MemoryFile_Align(file, 4);
            if(result == MEMORYFILE_OK)
                result = MemoryFile_WriteFloat(file, data->uvSet.sets[i].y);
        }
        break;
    case 14:
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteUInt16(file, data->part_val.shortVal);
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUTF8NullTerminatedString(file, data->part_val.stringVal, strlen(data->part_val.stringVal));  
        break;
    case 15:
        MemoryFile_Align(file, 2);
        result = MemoryFile_WriteUInt16(file, data->material_val.shortVal);
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUTF8NullTerminatedString(file, data->material_val.stringVal0, strlen(data->material_val.stringVal0));
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUTF8NullTerminatedString(file, data->material_val.stringVal1, strlen(data->material_val.stringVal1));
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUTF8NullTerminatedString(file, data->material_val.stringVal1, strlen(data->material_val.stringVal1));
        break;
    case 17:
        MemoryFile_Align(file, 4);
        result = MemoryFile_WriteInt32(file, data->model_bone_val.index);
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteInt32(file, data->model_bone_val.parent_index);
        MemoryFile_Align(file, 4);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUTF8NullTerminatedString(file, data->model_bone_val.name, strlen(data->model_bone_val.name));
        break;
    case 18:
        result = MemoryFile_WriteUInt8(file, data->tri_val.material_index);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUInt8(file, data->tri_val.object_index);
        break;
    case 19:
        result = MemoryFile_WriteUInt16(file, data->tri16_val.material_index);
        if(result == MEMORYFILE_OK)
            result = MemoryFile_WriteUInt16(file, data->tri16_val.object_index);
        break;
    default:
        return -2;
    }

    return result == MEMORYFILE_OK;
}