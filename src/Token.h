#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED
#include "Common.h"

typedef struct
{
  const char *name;
  uint32_t data_type;
  uint32_t index;
  uint32_t name_len;
  uint32_t hash;
} token_t;

typedef struct
{
    int index;
    int parent_index;
    char* name;
} tokenDataModelBoneDef_t;

typedef struct
{
    unsigned short index;
    float weight;
} tokenDataIndexWeight_t;

typedef struct
{
    unsigned short shortVal;
    char* stringVal;
} tokenDataPartDef_t;

typedef struct
{
    unsigned short shortVal;
    char* stringVal0;
    char* stringVal1;
    char* stringVal2;
} tokenDataMaterialDef_t;

typedef struct
{
    unsigned char material_index;
    unsigned char object_index;
    unsigned char unused_val0;
    unsigned char unused_val1;
} tokenDataTri_t;

typedef struct
{
    unsigned short material_index;
    unsigned short object_index;
    unsigned short unused_val0;
    unsigned short unused_val1;
} tokenDataTri16_t;

typedef struct
{
    union
    {
        struct
        {
            float x;
            float y;
        };
        float v[2];
    };
    
} vec2_t;

typedef struct
{
    unsigned short elem_count;
    vec2_t sets[16];
} uvSet_t;

typedef struct
{
    token_t token;

    union
    {
        int16_t shortVal;
        int32_t intVal;
        int64_t longVal;
        uint16_t ushortVal;
        uint32_t uintVal;
        uint64_t ulongVal;
        float floatVal;
        double doubleVal;
        tokenDataModelBoneDef_t model_bone_val;
        tokenDataIndexWeight_t bone_weight_val;
        tokenDataPartDef_t part_val;
        tokenDataMaterialDef_t material_val;
        tokenDataTri_t tri_val;
        tokenDataTri16_t tri16_val;
        float vec2Val[2];
        float vec3Val[3];
        float vec4Val[4];
        uvSet_t uvSet;
        char* stringVal;
    };
    
} tokenData_t;

int Token_TryFindFromCRC(uint16_t crc, tokenData_t* data);

int Token_TryFindFromName(const char* name, const long long name_len, tokenData_t* data);

int Token_TryGetNextToken(tokenData_t* data);

void Token_FreeData(tokenData_t* data);
#endif