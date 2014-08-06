/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 * 
 *  !!!!!THIS IS A PROTOTYPE AND NEEDS REWRITING!!!!!!!
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
#include <modloader_util_injector.hpp>
using namespace modloader;

#include "CAETrackLoader.h"
#include <list>
#include <map>


/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 52;
        
        // stream_import.ini
        struct StreamImport
        {
            std::string ini;        // 
            std::string folder;
            
            StreamImport(const std::string& ini, const std::string& folder)
            : ini(ini), folder(folder)
            {}
        };
        
        std::string strmpaks, traklkup;
        
        std::map<std::string, std::string> streams;
        
        //      <TrackID, OggPath>
        std::map<uint16_t, std::string> tracks;
        
        std::list<StreamImport> stream_import;
        
        
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        
        bool OnStartup();
        bool OnShutdown();
        
        bool CheckFile(modloader::modloader::file& file);
        bool ProcessFile(const modloader::modloader::file& file);
        bool PosProcess();
        
        //bool OnSplash();
        //bool OnLoad();
        //bool OnReload();
        
        const char** GetExtensionTable();
        
        
        
        void InitialiseTracks(CAETrackLoader&);

} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    modloader::RegisterPluginData(plugin, data, plugin.default_priority);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-tracks";
}

const char* CThePlugin::GetAuthor()
{
    return "LINK/2012";
}

const char* CThePlugin::GetVersion()
{
    return "0.1";
}

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { "ini", "dat", "", 0 };
    return table;
}

/*
 *  Startup / Shutdown (do nothing)
 */
bool CThePlugin::OnStartup()
{
    return true;
}

bool CThePlugin::OnShutdown()
{
    return true;
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::modloader::file& file)
{
    if(!file.is_dir)
    {
        if(*file.filext == 0)
        {
            if(IsFileInsideFolder(file.filepath, true, "STREAMS")) return true;
            if(!strcmp(file.filename, "AA", false)) return true;
            if(!strcmp(file.filename, "ADVERTS", false)) return true;
            if(!strcmp(file.filename, "AMBIENCE", false)) return true;
            if(!strcmp(file.filename, "CUTSCENE", false)) return true;
            if(!strcmp(file.filename, "BEATS", false)) return true;
            if(!strcmp(file.filename, "CH", false)) return true;
            if(!strcmp(file.filename, "CO", false)) return true;
            if(!strcmp(file.filename, "CR", false)) return true;
            if(!strcmp(file.filename, "DS", false)) return true;
            if(!strcmp(file.filename, "HC", false)) return true;
            if(!strcmp(file.filename, "MH", false)) return true;
            if(!strcmp(file.filename, "MR", false)) return true;
            if(!strcmp(file.filename, "NJ", false)) return true;
            if(!strcmp(file.filename, "RE", false)) return true;
            if(!strcmp(file.filename, "RG", false)) return true;
            if(!strcmp(file.filename, "TK", false)) return true;
        }
        else if(IsFileExtension(file.filext, "ini") && !strcmp(file.filename, "stream_import.ini", false))
            return true;
        else if(IsFileExtension(file.filext, "dat"))
        {
            if(!strcmp(file.filename, "TRAKLKUP.dat", false))
                return true;
            else if(!strcmp(file.filename, "STRMPAKS.dat", false))
                return true;
        }
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::modloader::file& file)
{
    if(*file.filext == 0)
    {
        std::string filename = file.filename; tolower(filename);
        std::string filepath = GetFilePath(file);
        
        auto it = this->streams.find(filename);
        if(it != this->streams.end())
            it->second = filepath;
        else
            this->streams.emplace(filename, filepath);
        
        return true;
    }
    else if(IsFileExtension(file.filext, "ini") && !strcmp(file.filename, "stream_import.ini", false))
    {
        std::string ini = GetFilePath(file);
        this->stream_import.emplace_front(ini, ini.substr(0, GetLastPathComponent(ini)));
        return true;
    }
    else if(IsFileExtension(file.filext, "dat"))
    {
        if(!strcmp(file.filename, "TRAKLKUP.dat", false))
            RegisterReplacementFile(*this, "TRAKLKUP.dat", this->traklkup, GetFilePath(file).c_str());
        else if(!strcmp(file.filename, "STRMPAKS.dat", false))
            RegisterReplacementFile(*this, "STRMPAKS.dat", this->strmpaks, GetFilePath(file).c_str());
        return true;
    }
    return false;
}

char* __stdcall transform_path1(char* dest, const char* src)
{
    std::string filename = src; tolower(filename);
    auto it = plugin.streams.find(filename);
    if(it != plugin.streams.end())
    {
        *dest = 0;
        src = it->second.c_str();
    }
    return strcat(dest, src);
}


/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    //.text:004DC620     ; CAEDataStream *__thiscall CAEDataStream__constructor(CAEDataStream *this, int id, char *path, unsigned int offset, unsigned int size, char encrypted)
    typedef function_hooker_fastcall<0x4E0E25, void*(void*, int, int, const char*, uint32_t, uint32_t, char)> ds_hook;
    typedef function_hooker<0x4E0DA2, void*(size_t)> a1_hook;
    typedef function_hooker<0x4E0AF2, void*(size_t)> a2_hook;

    typedef function_hooker_fastcall<0x4F1731, char(CAETrackLoader*)> init_hook;
    
    
    static void* transform_path = (void*)(transform_path1);
    
    auto alloc_path = [](a1_hook::func_type alloc, size_t& size)
    {
        if(size < MAX_PATH) size = MAX_PATH;
        return alloc(size);
    };
    

    make_function_hook<a1_hook>(alloc_path);
    make_function_hook<a2_hook>(alloc_path);
    WriteMemory(0x4E0DA7 + 2, &transform_path, true);
    WriteMemory(0x4E0AF7 + 2, &transform_path, true);
    
    
    make_function_hook<ds_hook>([](ds_hook::func_type func, void*& self, int&, int& id, const char*& szPath, uint32_t& dwOffset, uint32_t& dwSize, char& bEncrypted)
    {
        auto it = plugin.tracks.find(id);
        if(it != plugin.tracks.end())
        {
            szPath = it->second.c_str();
            dwOffset = dwSize = 0;
            bEncrypted = false;
        }
        return func(self, 0, id, szPath, dwOffset, dwSize, bEncrypted);
    });
    
    make_function_hook<init_hook>([](init_hook::func_type func, CAETrackLoader*& AETrackLoader)
    {
        char result = func(AETrackLoader);
        if(result) plugin.InitialiseTracks(*AETrackLoader);
        return result;
    });
    
    
    
    if(this->strmpaks.size())
        WriteMemory<const char*>(0x4E0982 + 1, this->strmpaks.c_str(), true);
    if(this->traklkup.size())
        WriteMemory<const char*>(0x4E0A02 + 1, this->traklkup.c_str(), true);
    
    return true;
}





void CThePlugin::InitialiseTracks(CAETrackLoader& AETrackLoader)
{
    
}





