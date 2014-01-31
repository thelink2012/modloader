/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include "bank.h"
#include "CWaveLoader.hpp"
#include <modloader_util.hpp>
#include <modloader_util_path.hpp>
using namespace modloader;

static CThePlugin plugin;
CThePlugin* bankPlugin;



/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    bankPlugin = &plugin;
    modloader::RegisterPluginData(plugin, data, plugin.default_priority);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    return "std-bank";
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
    static const char* table[] = { "wav", "dat", "", 0 };
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
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    if(!file.is_dir)
    {
        if(IsFileExtension(file.filext, "wav"))
            return true;
        else if(*file.filext == 0)  // No extension, may be a pak
        {
            if(IsFileInsideFolder(file.filepath, true, "SFX")) return true;
            if(!strcmp(file.filename, "FEET", false)) return true;
            if(!strcmp(file.filename, "GENRL", false)) return true;
            if(!strcmp(file.filename, "PAIN_A", false)) return true;
            if(!strcmp(file.filename, "SCRIPT", false)) return true;
            if(!strcmp(file.filename, "SPC_EA", false)) return true;
            if(!strcmp(file.filename, "SPC_FA", false)) return true;
            if(!strcmp(file.filename, "SPC_GA", false)) return true;
            if(!strcmp(file.filename, "SPC_NA", false)) return true;
            if(!strcmp(file.filename, "SPC_PA", false)) return true;
        }
        else if(IsFileExtension(file.filext, "dat"))
        {
            if(!strcmp(file.filename, "BankLkup.dat", false)) return true;
            if(!strcmp(file.filename, "BankSlot.dat", false)) return true;
            if(!strcmp(file.filename, "EventVol.dat", false)) return true;
            if(!strcmp(file.filename, "PakFiles.dat", false)) return true;
        }
    }
    return false;
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const ModLoaderFile& file)
{
    if(*file.filext == 0)   // No extension, is a pak
    {
        return this->AddPak(GetFilePath(file), file.filename);
    }
    else if(IsFileExtension(file.filext, "wav"))
    {
        CWavePCM wave(file.filepath);
        if(wave.IsOpen())
        {
            if(wave.GetNumChannels() == 1 && wave.GetBPS() == 16)
                return this->AddWave(file);
            else
                Log("Warning: Wave file \"%s\" is not 'mono PCM-16'", GetFilePath(file).c_str());
        }
    }
    else if(IsFileExtension(file.filext, "dat"))
    {
        std::string* buf = 0;
        
        if(!strcmp(file.filename, "BankLkup.dat", false)) buf = &BankLkup;
        else if(!strcmp(file.filename, "BankSlot.dat", false)) buf = &BankSlot;
        else if(!strcmp(file.filename, "EventVol.dat", false)) buf = &EventVol;
        else if(!strcmp(file.filename, "PakFiles.dat", false)) buf = &PakFiles;
        else return false;
        
        RegisterReplacementFile(*this, file.filename, *buf, GetFilePath(file).c_str());
        return true;
    }
    return false;
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    this->Patch();
    return true;
}


bool CThePlugin::AddWave(const modloader::ModLoaderFile& file)
{
    waves_pre.emplace_back(modloader::NormalizePath(file.filepath), modloader::GetFilePath(file));
    return true;
}

bool CThePlugin::AddWave(const WaveInfo& w)
{
    const std::string& filepath = w.sfx_path;
    const std::string& path = w.path;

    char pakfile[64];
    int bank, sound;

    if(sscanf(filepath.c_str() + GetLastPathComponent(filepath, 3), "%[^\\]\\bank_%d\\sound_%d.wav", pakfile, &bank, &sound) != 3)
    {
        if(sscanf(filepath.c_str() + GetLastPathComponent(filepath, 2), "bank_%d\\sound_%d.wav", &bank, &sound) == 2)
            *pakfile = 0;
        else
            return false;
    }
    
    if(*pakfile == 0 || !HasSFXPak(pakfile))
    {
        strcpy(pakfile, "GENRL");
        Log("Wave file \"%s\" not inside a SFXPak folder, assuming SFXPak %s", path.c_str(), pakfile);
    }
    
    if(bank > 0 && sound > 0)
    {
        AddWave(path, pakfile, bank - 1, sound - 1);
        return true;
    }

    return false;
}
