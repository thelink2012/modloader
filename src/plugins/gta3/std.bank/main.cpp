/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "CAECustomBankLoader.hpp"

// TODO cue points for sound loop offset

/*
 *  The plugin object
 */
class ThePlugin : public modloader::basic_plugin
{
    private:
        modloader::file_overrider ov_banklkup;    // BankLkup.dat overrider
        modloader::file_overrider ov_bankslot;    // BankSlot.dat overrider
        modloader::file_overrider ov_pakfiles;    // PakFiles.dat overrider
        modloader::file_overrider ov_eventvol;    // EventVol.dat overrider

    public:
        const info& GetInfo();

        bool OnStartup();
        bool OnShutdown();
        int GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        void Update();
        
} plugin;

REGISTER_ML_PLUGIN(::plugin);

/*
 *  ThePlugin::GetInfo
 *      Returns information about this plugin 
 */
const ThePlugin::info& ThePlugin::GetInfo()
{
    using namespace modloader;
    static const char* extable[] = { "", "dat", "wav", 0 };
    static const info xinfo      = { "std.bank", get_version_by_date(), "LINK/2012", -1, extable };
    return xinfo;
}


/*
 *  ThePlugin::OnStartup
 *      Startups the plugin
 */
bool ThePlugin::OnStartup()
{
    using namespace modloader;
    if(gvm.IsSA())
    {
        // Setup overrider parameters...
        // It's not possible to reinstall those config files, too much trouble
        auto no_reinstall = file_overrider::params(nullptr);
        ov_banklkup.SetParams(no_reinstall);
        ov_bankslot.SetParams(no_reinstall);
        ov_pakfiles.SetParams(no_reinstall);
        ov_eventvol.SetParams(no_reinstall);

        // Setup detourers for overriders...
        ov_banklkup.SetFileDetour(OpenFileDetour<0x4DFBDE>());
        ov_bankslot.SetFileDetour(OpenFileDetour<0x4E059E>());
        ov_pakfiles.SetFileDetour(OpenFileDetour<0x4DFC84>());
        ov_eventvol.SetFileDetour(OpenFileDetour<0x5B9D72>());

        // Patch the bank loader
        CAbstractBankLoader::Patch();

        return true;
    }
    return false;
}

/*
 *  ThePlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool ThePlugin::OnShutdown()
{
    return true;
}

/*
 *  ThePlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int ThePlugin::GetBehaviour(modloader::file& file)
{
    if(!file.is_dir())
    {
        if(file.is_ext(""))
        {
            // Accept any file without extension in a SFX/ folder as a SFXPak
            // Also accept default SFXPaks outside that folder....
            if(modloader::IsFileInsideFolder(file.filedir(), true, "SFX")
            || !_stricmp(file.filename(), "FEET")   || !_stricmp(file.filename(), "GENRL")
            || !_stricmp(file.filename(), "PAIN_A") || !_stricmp(file.filename(), "SCRIPT")
            || !_stricmp(file.filename(), "SPC_EA") || !_stricmp(file.filename(), "SPC_FA")
            || !_stricmp(file.filename(), "SPC_GA") || !_stricmp(file.filename(), "SPC_NA")
            || !_stricmp(file.filename(), "SPC_PA"))
            {
                file.behaviour = SetType(file.hash, Type::SFXPak);
                return MODLOADER_BEHAVIOUR_YES;
            }
        }
        else if(file.is_ext("dat"))
        {
            // Data files hashes, must be lower case (normalized), to check against file hash
            static const auto banklkup_dat = modloader::hash("banklkup.dat");
            static const auto bankslot_dat = modloader::hash("bankslot.dat");
            static const auto eventvol_dat = modloader::hash("eventvol.dat");
            static const auto pakfiles_dat = modloader::hash("pakfiles.dat");

            // Check if this is an bank loader data file by checking it's filename hash
            if(file.hash == banklkup_dat)
                file.behaviour = SetType(file.hash, Type::BankLookUp);
            else if(file.hash == bankslot_dat)
                file.behaviour = SetType(file.hash, Type::BankSlot);
            else if(file.hash == eventvol_dat)
                file.behaviour = SetType(file.hash, Type::EventVol);
            else if(file.hash == pakfiles_dat)
                file.behaviour = SetType(file.hash, Type::PakFiles);
            else
                return MODLOADER_BEHAVIOUR_NO;

            return MODLOADER_BEHAVIOUR_YES;
        }
        else if(file.is_ext("wav"))
        {
            // Cool, this is a wave file, let's check if it's compatible with San Andreas
            CWavePCM wave(file.fullpath().c_str());
        
            if(wave.HasChunks())
            {
                // San Andreas accept only mono PCM 16 wave files.....
                if(wave.GetNumChannels() == 1 && wave.GetBPS() == 16)
                {
                    using modloader::GetLastPathComponent;
                    using modloader::NormalizePath;

                    char pakfile[64]; unsigned int bank, sound;     //
                    auto filedir = NormalizePath(file.filedir());   // Lower case, using '\\' as path separator

                    // Try "PAKFILE/BANK_N/SOUND_N" pattern on the path, to find the wave bank and sound
                    if(sscanf(&filedir[GetLastPathComponent(filedir, 3)], "%[^\\]\\bank_%u\\sound_%u.wav", pakfile, &bank, &sound) != 3)
                    {
                        // Nope, so let's try just "BANK_N/SOUND_N" pattern, and assume GENRL as the bank
                        if(sscanf(&filedir[GetLastPathComponent(filedir, 2)], "bank_%u\\sound_%u.wav", &bank, &sound) == 2)
                            strcpy(pakfile, "genrl");
                        else
                            return MODLOADER_BEHAVIOUR_NO;
                    }

                    // Set behaviour according to the gathered information on the file path
                    file.behaviour = SetType(modloader::hash(pakfile), Type::Wave);     // Wave file
                    file.behaviour = SetBank(SetSound(file.behaviour, sound), bank);    // Bank / Sound
                    return MODLOADER_BEHAVIOUR_YES;
                }
                else
                    Log("Warning: Wave file \"%s\" is not mono PCM-16, ignoring it!", file.filepath());
            }
            else
                Log("Warning: Invalid or bad formatted wave file \"%s\"", file.filepath());

        }
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  ThePlugin::InstallFile
 *      Installs a file using this plugin
 */
bool ThePlugin::InstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::Wave:        return banker.InstallWave(file);
        case Type::SFXPak:      return banker.AddSfxPak(file);
        case Type::BankLookUp:  return ov_banklkup.InstallFile(file);
        case Type::BankSlot:    return ov_bankslot.InstallFile(file);
        case Type::PakFiles:    return ov_pakfiles.InstallFile(file);
        case Type::EventVol:    return ov_eventvol.InstallFile(file);
    }
    return false;
}

/*
 *  ThePlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool ThePlugin::ReinstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::Wave:        return banker.ReinstallWave(file);
        case Type::SFXPak:      return true;    // Avoid catastrophical failure
        case Type::BankLookUp:  return ov_banklkup.ReinstallFile();
        case Type::BankSlot:    return ov_bankslot.ReinstallFile();
        case Type::PakFiles:    return ov_pakfiles.ReinstallFile();
        case Type::EventVol:    return ov_eventvol.ReinstallFile();
    }
    return false;
}

/*
 *  ThePlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool ThePlugin::UninstallFile(const modloader::file& file)
{
    switch(GetType(file.behaviour))
    {
        case Type::Wave:        return banker.UninstallWave(file);
        case Type::SFXPak:      return banker.RemSfxPak(file);
        case Type::BankLookUp:  return ov_banklkup.UninstallFile();
        case Type::BankSlot:    return ov_bankslot.UninstallFile();
        case Type::PakFiles:    return ov_pakfiles.UninstallFile();
        case Type::EventVol:    return ov_eventvol.UninstallFile();
    }
    return false;
}

/*
 *  ThePlugin::Update
 *      Updates the state of this plugin after a serie of install/uninstalls
 */
void ThePlugin::Update()
{
    // Refresh the sounds
    banker.Update();
}
