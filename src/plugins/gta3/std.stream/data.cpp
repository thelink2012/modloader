/*
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 *
 */
#include <stdinc.hpp>
#include "streaming.hpp"
using namespace std::placeholders;

extern "C"
{
    void* (*ColModelPool_new)(int)  = nullptr;
    extern void HOOK_LoadColFileFix(int);
};

static void FixColFile();
static void* LoadNonStreamedRes(std::function<void*(const char*)> load, const char* filepath, NonStreamedType type);

/*
 *  gta.dat / default.dat related stuff
 */
void CAbstractStreaming::DataPatch()
{
    if(gvm.IsSA())
        FixColFile(); // Fix broken COLFILE in GTA SA


    // Detouring of non streamed resources loaded by gta.dat/default.dat
    if(true)
    {
        using hcolfile  = function_hooker<0x5B9188, void*(const char*, int)>;
        using hatomic   = function_hooker<0x5B91B0, void*(const char*)>;
        using hclump    = function_hooker<0x5B91DB, void*(const char*)>;
        using htexdict  = function_hooker<0x5B910A, void*(const char*)>;

        make_static_hook<hatomic>(std::bind(LoadNonStreamedRes, _1, _2, NonStreamedType::AtomicFile));
        make_static_hook<hclump>(std::bind(LoadNonStreamedRes, _1, _2, NonStreamedType::ClumpFile));
        make_static_hook<htexdict>(std::bind(LoadNonStreamedRes, _1, _2, NonStreamedType::TexDictionary));
        make_static_hook<hcolfile>([](hcolfile::func_type load, const char* filepath, int id)
        {
            return LoadNonStreamedRes(std::bind(load, _1, id), filepath, NonStreamedType::ColFile);
        });
    }
}


/*
 *  CAbstractStreaming::TryLoadNonStreamedResource
 *      Checks if the file 'filepath' is under our ownership, and if it is register as non-streamed.
 */
std::string CAbstractStreaming::TryLoadNonStreamedResource(std::string filepath, NonStreamedType type)
{
    if(!this->bHasInitializedStreaming)
    {
        auto filename = NormalizePath(filepath.substr(GetLastPathComponent(filepath)));

        auto it = this->raw_models.find(filename);
        if(it != this->raw_models.end())
        {
            // Log about non streamed resource and make sure it's unique
            plugin_ptr->Log("Using non-streamed resource \"%s\"", it->second->filepath());
            if(this->non_stream.count(it->second->hash))
                throw std::runtime_error("std.stream: TryLoadNonStreamedResource: Repeated resource!");

            // Register into non_stream and unregister from raw_models
            this->non_stream.emplace(it->second->hash, std::make_pair(it->second, type));
            std::string fullpath = it->second->fullpath();
            this->raw_models.erase(it);
            return fullpath;
        }
    }
    return std::string();
}

/*
 *  LoadNonStreamedRes
 *      Direct wrapper between the game calls and CAbstractStreaming::TryLoadNonStreamedResource 
 */
void* LoadNonStreamedRes(std::function<void*(const char*)> load, const char* filepath, NonStreamedType type)
{
    auto newfilepath = streaming.TryLoadNonStreamedResource(filepath, type);
    return load(newfilepath.size()? newfilepath.data() : filepath);
}


/*
 *  FixColFile
 *      Fixes the COLFILE from gta.dat not working properly, crashing the game.
 */
void FixColFile()
{
    static bool using_colbuf;           // Is using colbuf or original buffer?
    static bool empty_colmodel;         // Is this a empty colmodel?
    static std::vector<char> colbuf;    // Buffer for reading col file into

    // Prototype for patches
    using rcolinfo_f  = int(void*, uint32_t*, size_t);
    using rcolmodel_f = int(void*, char*, size_t);
    using lcol_f      = int(char*, int, void*, char*);
    using rel_f       = int(void*);

    // Fixes the crash caused by using COLFILE for a building etc
    ColModelPool_new = MakeCALL(0x5B4F2E, HOOK_LoadColFileFix).get();

    // Reads collision info and check if we need to use our own collision buffer
    auto ReadColInfo = [](std::function<rcolinfo_f> Read, void*& f, uint32_t*& buffer, size_t& size)
    {
        auto r    = Read(f, buffer, size);
        empty_colmodel = (buffer[1] <= 0x18);
        if(using_colbuf = !empty_colmodel)
            colbuf.resize(buffer[1]);
        return r;
    };

    // Replace buffer if necessary
    auto ReadColModel = [](std::function<rcolmodel_f> Read, void*& f, char*& buffer, size_t& size)
    {
        if(!empty_colmodel)
            return Read(f, using_colbuf? colbuf.data() : buffer, size);
        return 0;
    };

    // Replace buffer if necessary
    auto LoadCollisionModel = [](std::function<lcol_f> LoadColModel, char*& buf, int& size, void*& colmodel, char*& modelname)
    {
        return LoadColModel(using_colbuf? colbuf.data() : buf, size, colmodel, modelname);
    };

    // Frees our buffer
    auto ReleaseBuffer = [](std::function<rel_f> fclose, void*& f)
    {
        colbuf.clear(); colbuf.shrink_to_fit();
        return fclose(f);
    };

    // Patches
    make_static_hook<function_hooker<0x5B4EF4, rcolmodel_f>>(ReadColModel);
    make_static_hook<function_hooker<0x5B4E92, rcolinfo_f>>(ReadColInfo);
    make_static_hook<function_hooker<0x5B4FCC, rcolinfo_f>>(ReadColInfo);
    make_static_hook<function_hooker<0x5B4FA0, lcol_f>>(LoadCollisionModel);
    make_static_hook<function_hooker<0x5B4FB5, lcol_f>>(LoadCollisionModel);
    make_static_hook<function_hooker<0x5B4F83, lcol_f>>(LoadCollisionModel);
    make_static_hook<function_hooker<0x5B92F9, rel_f>>(ReleaseBuffer);
}
