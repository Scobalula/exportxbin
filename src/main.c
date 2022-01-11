#include "Common.h"
#include "Globals.h"
#include "Utility.h"
#include "Version.h"
#include "Converter.h"
#include "FileName.h"
#include "FileSystem.h"

// TODO: Better error reporting in some places
// TODO: Cleanup and run some validations

void ProcessDirScanFile(const char* dir, const char* file_name)
{
    char full_path[260];

    int p_result = sprintf_s(full_path, sizeof(full_path), "%s\\%s", dir, file_name);

    if(FileName_HasExtension(full_path, ".xmodel_bin") || FileName_HasExtension(full_path, ".xanim_bin"))
    {
        if(Global_AllowBinDirScan)
        {
            Converter_AddItem(full_path, CONVERT_TYPE_BIN_TO_EXPORT);
        }
    }
    if(FileName_HasExtension(full_path, ".xmodel_export") || FileName_HasExtension(full_path, ".xanim_export"))
    {
        if(Global_AllowExportDirScan)
        {
            Converter_AddItem(full_path, CONVERT_TYPE_EXPORT_TO_BIN);
        }
    }
}

void ProcessArgs(int argc, char* argv[])
{
    Converter_Init(Global_ThreadCount);

    for(int i = 1; i < argc; i++)
    {
        if(FileName_HasExtension(argv[i], ".xmodel_bin") || FileName_HasExtension(argv[i], ".xanim_bin"))
        {
            Converter_AddItem(argv[i], CONVERT_TYPE_BIN_TO_EXPORT);
        }
        if(FileName_HasExtension(argv[i], ".xmodel_export") || FileName_HasExtension(argv[i], ".xanim_export"))
        {
            Converter_AddItem(argv[i], CONVERT_TYPE_EXPORT_TO_BIN);
        }
        else if(FileSystem_IsDirectory(argv[i]))
        {
            FileSystem_EnumerateFiles(argv[i], 1, "*.*", ProcessDirScanFile);
        }
    }
}

void TrimInput(char* v, size_t max_len)
{
    int string_len = strnlen_s(v, max_len);
    int actual_start = -1;
    int actual_end = -1;

    // Try to find first and last instance
    // of something that isn't a quote
    for (size_t i = 0; i < string_len; i++)
    {
        if(v[i] != ' ' && v[i] != '"' && v[i] != '\n' && v[i] != '\r' && v[i] != '\t')
        {
            actual_start = i;
            break;
        }
    }
    for (size_t i = string_len - 1; i >= 0; i--)
    {
        if(v[i] != ' ' && v[i] != '"' && v[i] != '\n' && v[i] != '\r' && v[i] != '\t')
        {
            actual_end = i + 1;
            break;
        }
    }

    if(actual_start == -1)
    {
        return;
    }
    if(actual_end == -1)
    {
        return;
    }

    int actual_size = actual_end - actual_start;

    if(actual_size <= 0)
    {
        return;
    }

    memcpy(&v[0], &v[actual_start], actual_size);
    v[actual_size] = '\0';
}

void ProcessPiped()
{
    char buffer[1024];
    Converter_Init(1);
    printf("| Running piped mode.\n");

    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        printf("| Enter Path: ");
        fgets(buffer, sizeof(buffer), stdin);
        TrimInput(buffer, sizeof(buffer));

        if(!strncmp(buffer, "exit", 4))
        {
            printf("| Exiting piped mode.\n");
            break;
        }
        else if(FileName_HasExtension(buffer, ".xmodel_bin") || FileName_HasExtension(buffer, ".xanim_bin"))
        {
            Converter_AddItem(buffer, CONVERT_TYPE_BIN_TO_EXPORT);
        }
        else if(FileName_HasExtension(buffer, ".xmodel_export") || FileName_HasExtension(buffer, ".xanim_export"))
        {
            Converter_AddItem(buffer, CONVERT_TYPE_EXPORT_TO_BIN);
        }
        else
        {
            printf("| ERROR: Invalid arg entered.\n");
        }
    }
}

int main(int argc, char* argv[])
{
    printf("| -----------------------------------------\n");
    printf("| exportxbin - by Scobalula\n");
    printf("| Converts EXPORT to/from BIN\n");
    printf("| Version: %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUGFIX);
    printf("| -----------------------------------------\n");

    if(argc < 2)
    {
        printf("| To use, simply drag and drop files/folders.\n");
        printf("| Or, pass /piped to enter files on the fly.\n|\n");
    }
    else
    {
        // First pass for settings
        for (int i = 0; i < argc; i++)
        {
            if (!stricmp(argv[i], "/piped"))
            {
                Global_Mode = 1;
            }
            if (!stricmp(argv[i], "/dir_allowxbin"))
            {
                Global_AllowBinDirScan = 1;
            }
        }
        
        switch (Global_Mode)
        {
        case 0:
            ProcessArgs(argc, argv);
            break;
        case 1:
            ProcessPiped();
            break;
        default:
            printf("| ERROR: Invalid mode.");
            break;
        }
        // Convert trailing files.
        Converter_Run();
        if(Converter_NumFilesProcessed() == 0)
        {
            printf("| No files processed.\n");
        }
    }
    
    Converter_Shutdown();
    Allocator_Report("MemoryDebug.txt");

    printf("| Execution complete, press Enter to exit.");
    getchar();

    return 0;
}