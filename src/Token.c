#include "Utility.h"
#include "Token.h"

token_t SupportedTokens[60] =
{
  { ";", 0u, 0u, 1u, 34616u },
  { "//", 0u, 1u, 2u, 50005u },
  { "AMBIENTCOLOR", 11u, 2u, 12u, 14335u },
  { "ANIMATION", 1u, 3u, 9u, 31404u },
  { "BLINN", 9u, 4u, 5u, 33735u },
  { "BONE", 3u, 5u, 4u, 56730u },
  { "BONE", 12u, 6u, 4u, 61867u },
  { "BONE", 17u, 7u, 4u, 61593u },
  { "BONES", 3u, 8u, 5u, 59974u },
  { "COEFFS", 9u, 9u, 6u, 51253u },
  { "COLOR", 6u, 10u, 5u, 28120u },
  { "FIRSTFRAME", 3u, 11u, 10u, 48340u },
  { "FRAME", 4u, 12u, 5u, 50979u },
  { "FRAME", 16u, 13u, 5u, 5749u },
  { "FRAMERATE", 3u, 14u, 9u, 37587u },
  { "GLOW", 9u, 15u, 4u, 65036u },
  { "INCANDESCENCE", 11u, 16u, 13u, 16997u },
  { "MATERIAL", 15u, 17u, 8u, 42752u },
  { "MODEL", 1u, 18u, 5u, 18120u },
  { "NORMAL", 7u, 19u, 6u, 35308u },
  { "NOTETRACK", 3u, 20u, 9u, 17987u },
  { "NOTETRACKS", 1u, 21u, 10u, 51187u },
  { "NUMBONES", 3u, 22u, 8u, 30394u },
  { "NUMFACES", 5u, 23u, 8u, 48786u },
  { "NUMFRAMES", 5u, 24u, 9u, 47383u },
  { "NUMKEYS", 3u, 25u, 7u, 31340u },
  { "NUMMATERIALS", 3u, 26u, 12u, 41394u },
  { "NUMOBJECTS", 3u, 27u, 10u, 25263u },
  { "NUMPARTS", 3u, 28u, 8u, 37497u },
  { "NUMTRACKS", 3u, 29u, 9u, 36886u },
  { "NUMVERTS", 3u, 30u, 8u, 38157u },
  { "NUMVERTS32", 5u, 31u, 10u, 10988u },
  { "OBJECT", 14u, 32u, 6u, 34772u },
  { "OFFSET", 10u, 33u, 6u, 37763u },
  { "PART", 3u, 34u, 4u, 29786u },
  { "PART", 14u, 35u, 4u, 13835u },
  { "PHONG", 8u, 36u, 5u, 23762u },
  { "REFLECTIVE", 9u, 37u, 10u, 32118u },
  { "REFLECTIVECOLOR", 11u, 38u, 15u, 58771u },
  { "REFRACTIVE", 9u, 39u, 10u, 32292u },
  { "SCALE", 10u, 40u, 5u, 7254u },
  { "SPECULARCOLOR", 11u, 41u, 13u, 12668u },
  { "TRANSPARENCY", 11u, 42u, 12u, 28075u },
  { "TRI", 18u, 43u, 3u, 22063u },
  { "TRI16", 19u, 44u, 5u, 26385u },
  { "UV", 13u, 45u, 2u, 6868u },
  { "VERSION", 3u, 46u, 7u, 9425u },
  { "VERT", 3u, 47u, 4u, 36611u },
  { "VERT32", 5u, 48u, 6u, 45207u },
  { "X", 7u, 49u, 1u, 56573u },
  { "Y", 7u, 50u, 1u, 52444u },
  { "Z", 7u, 51u, 1u, 64703u },
  { "NUMSBONES", 5u, 52u, 9u, 8130u },
  { "NUMSWEIGHTS", 5u, 53u, 11u, 45918u },
  { "QUATERNION", 11u, 54u, 10u, 61289u },
  { "NUMIKPITCHLAYERS", 5u, 55u, 16u, 42587u },
  { "IKPITCHLAYER", 4u, 56u, 12u, 7549u },
  { "ROTATION", 10u, 57u, 8u, 42379u },
  { "NUMCOSMETICBONES", 5u, 58u, 16u, 30774u },
  { "EXTRA", 11u, 59u, 5u, 28398u }
};

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

int Token_TryFindFromCRC(uint16_t crc, tokenData_t* data)
{
    for(size_t i = 0; i < ARRAY_SIZE(SupportedTokens); i++)
    {
        if(SupportedTokens[i].hash == crc)
        {
            if(data != NULL)
            {
                data->token = SupportedTokens[i];
            }
            return 1;
        }
    }

    if(data != NULL)
    {
        data->token.hash = crc;
    }

    return 0;
}

int Token_TryFindFromName(const char* name, const long long name_len, tokenData_t* data)
{
    for(size_t i = 0; i < ARRAY_SIZE(SupportedTokens); i++)
    {
        int length = name_len == -1 ? SupportedTokens[i].name_len : name_len;
        
        if(!strncmp(name, SupportedTokens[i].name, length))
        {
            if(data != NULL)
            {
                data->token = SupportedTokens[i];
            }

            return 1;
        }
    }

    if(data != NULL)
    {
        data->token.name = name;
        data->token.name_len = name_len;
    }

    return 0;
}

int Token_TryGetNextToken(tokenData_t* data)
{
    size_t max_items = ARRAY_SIZE(SupportedTokens);
    size_t next_index = data->token.index + 1;

    if(next_index >= max_items)
    {
        return 0;
    }
    if(data->token.name_len != SupportedTokens[next_index].name_len)
    {
        return 0;
    } 
    if(strcmp(data->token.name, SupportedTokens[next_index].name))
    {
        return 0;
    }

    data->token = SupportedTokens[next_index];
    return 1; 
}

void Token_FreeData(tokenData_t* data)
{
    switch (data->token.data_type)
    {
    case 0:
        free(data->stringVal);
        break;      
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        break;
    case 10:
        break;
    case 11:
        break;
    case 12:
        break;
    case 13:
        break;
    case 14:
        free(data->part_val.stringVal);
        break;
    case 15:
        free(data->material_val.stringVal0);
        free(data->material_val.stringVal1);
        free(data->material_val.stringVal2);
        break;
    case 17:
        free(data->model_bone_val.name);
        break;
    case 18:
        break;
    case 19:
        break;
    default:
        break;
    }

    memset(data, 0, sizeof(tokenData_t));
}