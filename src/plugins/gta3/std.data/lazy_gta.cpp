/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
using namespace modloader;
using namespace injector;
using namespace std::placeholders;

/*
 *   This file adds lazy loading of default.dat and gta.dat
 *   What that effectively means is that we can fix the loading screen flickering bug, caused by the amount of entries in default/gta.dat
 *   We can also deal with some post processing of entries based on std.data virtual file system.
 */

void ProcessGtaDatEntries();

enum class LineType
{
    // Must be ordered by order-to-be-loaded (matters)
    IMG, TEXDICTION, MODELFILE, IDE, COLFILE, IPL, HIERFILE, SPLASH, EXIT
};

// NOTICE MUST HAVE THE SAME ORDERING AS THE ABOVE ENUM
std::pair<LineType, std::string> LineTypeAssoc[] =
{
    { LineType::IMG,        "IMG"           },                
    { LineType::TEXDICTION, "TEXDICTION"    },
    { LineType::MODELFILE,  "MODELFILE"     },
    { LineType::IDE,        "IDE"           },
    { LineType::COLFILE,    "COLFILE"       },
    { LineType::IPL,        "IPL"           },
    { LineType::HIERFILE,   "HIERFILE"      },
    { LineType::SPLASH,     "SPLASH"        },
    { LineType::EXIT,       "EXIT"          }
};

using lazydata = std::multimap<LineType, std::string>;
static lazydata entries;


/*
 *  FixLoadingScreen
 *      Fixes the loading bar flickering when we remove (or even add more?) entries
 *      The issue was, the game had a harded coded counter of expected entries in gtadat
 */
static void FixLoadingScreen()
{
    static const int default_amount_of_files = 110; // The game loads this number of entries that contribute to the loading bar by default

    int amount = std::count_if(entries.begin(), entries.end(), [](const lazydata::value_type& pair) {
        // those secs do not account for the loading bar progress
        return(pair.first != LineType::IMG && pair.first != LineType::SPLASH && pair.first != LineType::EXIT);
    });

    WriteMemory(0x590D2A+1, ReadMemory<int32_t>(0x590D2A+1, true) - default_amount_of_files + amount, true);
    WriteMemory(0x590D67+1, ReadMemory<int32_t>(0x590D67+1, true) - default_amount_of_files + amount, true);
}

/*
 *  PerformLoadLevel
 *      Performs an effective load level that let's the game load IDE, IPLs and so on.
 *      The data used by the load process is virtual created by us, let's make them believe it's true!!!
 */
static void PerformLoadLevel(std::function<void(const char*)> LoadLevel)
{
    plugin_ptr->Log("Loading preloaded level entries...");

    // Before doing any further processing we need to process the data we have
    // So we can add more entries or rearange something if needed
    ProcessGtaDatEntries();

    // Then fix the loading bar, we need to do so after we have everything processed and before actually loading the gtadata
    // The problem with the loading bar is the number of entries in the gtadat file!
    FixLoadingScreen();

    // Loads gtadat by sending to the game lines created by us by our preloaded data
    if(true)
    {
        using readln1_hook = function_hooker<0x5B906A, const char*(void*)>;
        using readln2_hook = function_hooker<0x5B92E6, const char*(void*)>;
        scoped_nop<5> nop_openfile(0x5B905E, 5);    // no open, we'll build the lines ourself
        scoped_nop<5> nop_closefile(0x5B92F9, 5);   // ...yeah

        auto it = entries.begin();
        std::string buffer;
        
        // Builds an line from the (current) next entry and returns it to the game so it can read it
        auto build_line = [&](readln1_hook::func_type, void*) -> const char*
        {
            if(it != entries.end())
            {
                buffer.assign(LineTypeAssoc[(int)it->first].second).append(" ");
                if(it->first == LineType::COLFILE) buffer.append("0 ");
                buffer.append(it->second);

                ++it;
                return buffer.data();
            }
            return nullptr;
        };

        // Forward to the game after placig our build line hooks :D
        auto readnl1 = make_function_hook<readln1_hook>(build_line);
        auto readnl2 = make_function_hook<readln2_hook>(build_line);
        LoadLevel(nullptr);
    }

    entries.clear();
}

/*
 *  PreLoadLevel
 *      Instead of effectively loading the level file, capture all it's entries to our entries map for later processing
 *      by PerformLoadLevel
 */
static void PreLoadLevel(std::function<void(const char*)> LoadLevel, const char* filename)
{
    using readln_hook = function_hooker<0x5B906A, char*(void*)>;

    plugin_ptr->Log("Preloading level file \"%s\"", filename);

    // Hooks the first call to LoadLine in the LoadLevel functor to do our proper reading and parsing
    // After that, return no character so the game itself can get out of the function without reading anything from the dat file
    auto readnl1 = make_function_hook<readln_hook>([&](readln_hook::func_type LoadLine, void* fhandle)
    {
        while(char* line = LoadLine(fhandle))
        {
            if(line[0] != '#')
            {
                auto it = std::find_if(std::begin(LineTypeAssoc), std::end(LineTypeAssoc), [&](const lazydata::value_type& pair) {
                    return strncmp(pair.second.data(), line, pair.second.length()) == 0;
                });

                if(it != std::end(LineTypeAssoc))
                {
                    if(it->first != LineType::EXIT)
                    {
                        auto skip = it->second.length() + (it->first == LineType::COLFILE? 3 : 1);
                        entries.emplace(it->first, modloader::NormalizePath(std::string(line + skip)));
                        continue;
                    }
                    break;
                }
            }
        }
        return nullptr;
    });

    return LoadLevel(filename);
}


/*
 *  ProcessGtaDatEntries
 *      This function looks at the entries we have preloaded from gtadat and checks if any of our files in DataPlugin filesystem
 *      is out of place or not even going to be loaded, if any of that happens, fix the place or make it load (adding a entry for it).
 */
void ProcessGtaDatEntries()
{
    using fs_value_type = decltype(DataPlugin::fs)::value_type;
    std::multimap<std::string, std::reference_wrapper<const std::string>> gtafiles;
    auto& fs = static_cast<DataPlugin*>(plugin_ptr)->fs;

    // Add the IDE and IPL entries to the gtafiles map so we can take care of them on here
    std::for_each(entries.begin(), entries.end(), [&](const lazydata::value_type& entry)
    {
        if(entry.first == LineType::IDE || entry.first == LineType::IPL)
            gtafiles.emplace(GetPathComponentBack(entry.second), std::cref(entry.second));
    });

    std::vector<std::pair<decltype(DataPlugin::fs)::iterator, std::string>> tomove;

    // Traverse our virtual filesystem checking for files not attached to any gtadat
    // (Either because the user didn't move the file to the right path or he missed the gtadat entry)
    for(auto it = fs.begin(); it != fs.end(); ++it)
    {
        auto& mfile  = it->second.second;
        auto& vpath = it->first;

        const bool is_ide = mfile->is_ext("ide");
        const bool is_ipl = !is_ide && (mfile->is_ext("ipl") ||  mfile->is_ext("zon"));

        if(is_ide || is_ipl)    // we are up to handle only IDE and IPLs processing like that
        {
            auto filename = GetPathComponentBack(vpath);
            auto count    = gtafiles.count(filename);

            if(count == 1)
            {
                // So, we have only file named after this vfs file in the gtadat entries
                // Check out if this file is attached to the correct virtual path, otherwise correct it
                auto& gtapath = gtafiles.find(filename)->second.get();
                if(gtapath != vpath)
                {
                    tomove.emplace_back(it, gtapath);   // lazly since adding something during a loop is bad
                    plugin_ptr->Log("The file \"%s\" does not seem to be in the right virtual path, moving from \"%s\" to \"%s\"",
                        mfile->filepath(), vpath.c_str(), gtapath.c_str());
                }
            }
            else if(count == 0)
            {
                // Missing this entry, let's add it
                if(false)
                {
                    entries.emplace(is_ide? LineType::IDE : LineType::IPL, vpath);
                    plugin_ptr->Log("The file \"%s\" is not a registered entry in the level file, registering it anyway as \"%s\"",
                        mfile->filepath(), vpath.c_str());
                }
                else
                {
                    plugin_ptr->Log("The file \"%s\" is not a registered entry in the level file, ignoring.",
                        mfile->filepath(), vpath.c_str());
                }
            }
        }
    }

    // Do actual move of files that needs to be realigned
    for(auto& x : tomove)
        fs.move_file(x.first, std::move(x.second));
}






/*
 *  LazyGtaDatPatch
 *      Call this to apply the patch this file is related to.
 *      No harm at all will be caused if this patch is not performed.
 */
void LazyGtaDatPatch()
{
    /// TODO VC III
    if(!gvm.IsSA())
        return;

    using defaultdat_hook = function_hooker<0x53BC95, void(const char*)>;
    using gtadat_hook     = function_hooker<0x53BC9B, void(const char*)>;
    make_static_hook<defaultdat_hook>(PreLoadLevel);
    make_static_hook<gtadat_hook>([](gtadat_hook::func_type LoadLevel, const char* filename)
    {
        PreLoadLevel(LoadLevel, filename);
        return PerformLoadLevel(LoadLevel);
    });
}
