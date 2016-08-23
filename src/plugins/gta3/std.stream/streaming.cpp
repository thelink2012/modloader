/*
 * Standard Streamer Plugin for Mod Loader
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 *
 */
#include <stdinc.hpp>
#include "streaming.hpp"
using namespace modloader;

CAbstractStreaming* streaming;


/*
 *  Constructs the abstract streaming object 
 */
CAbstractStreaming::CAbstractStreaming()
{
    InitializeCriticalSection(&cs);
}

CAbstractStreaming::~CAbstractStreaming()
{
    DeleteCriticalSection(&cs);
    Fastman92LimitAdjusterDestroy(this->f92la);
}

void CAbstractStreaming::InitialiseStructAbstraction()
{
	if(this->bIsFirstLaunched)
		return;		// We already init this once, no need to do again

    this->sizeof_CStreamingInfo = CStreamingInfo::GetSizeof();
    this->f92la                 = Fastman92LimitAdjusterCreate();

    if(this->f92la.hLib)
        plugin_ptr->Log("Using fastman92limitadjuster (%p).", f92la.hLib);
}

const LibF92LA& CAbstractStreaming::GetF92LA()
{
    return this->f92la;
}

const LibF92LA* CStreamingInfo::GetF92LA()
{
    auto& f92la = streaming->GetF92LA();
    return f92la.hLib? &f92la : nullptr;
}

int32_t CStreamingInfo::AsF92LA()
{
    return streaming->InfoForModelIndex(*this);
}


/*
 *  CAbstractStreaming::InfoForModel
 *      Returns the streaming info pointer for the specified resource id
 */
CStreamingInfo* CAbstractStreaming::InfoForModel(id_t id)
{
    // Note: sizeof(CStreamingInfo) isn't the actual size, so we must do the indexing manually with sizeof_CStreamingInfo!!
    static CStreamingInfo* max_InfoForModel = this->f92la.hLib?
                                                (CStreamingInfo*)((uint8_t*)(ms_aInfoForModel) + (f92la.GetNumberOfFileIDs() * sizeof_CStreamingInfo)) :
                                                lazy_object<0x5B8AFC, CStreamingInfo*>::get();
    CStreamingInfo* info = (CStreamingInfo*)((uint8_t*)(ms_aInfoForModel) + (id * sizeof_CStreamingInfo));
    return (info < max_InfoForModel? info : nullptr);
}

// Returns the resource index from it's model info structure
auto CAbstractStreaming::InfoForModelIndex(const CStreamingInfo& model) -> id_t
{
    // Note: sizeof(CStreamingInfo) isn't the actual size, so we must do the substraction manually with sizeof_CStreamingInfo!!
    return (id_t)( ((uint8_t*)(&model) - (uint8_t*)(InfoForModel(0))) / sizeof_CStreamingInfo );
}

/*
 *  CAbstractStreaming::IsModelOnStreaming
 *      Checks if the specified model is on the streaming, either on the bus to get loaded or already loaded.
 */
bool CAbstractStreaming::IsModelOnStreaming(id_t id)
{
    return InfoForModel(id)->GetLoadStatus() != 0;
}

/*
 *  CAbstractStreaming::IsModelAvailable
 *      Checks if the specified model is available to use (i.e. already loaded).
 */
bool CAbstractStreaming::IsModelAvailable(id_t id)
{
    return InfoForModel(id)->GetLoadStatus() == 1;
}

/*
 *  CAbstractStreaming::RequestModel
 *      Requests a resource into the streaming, essentially loading it
 *      Notice the model will not be available right after the call, it's necessary to wait for it.
 *      Flags are unknown... still researching about them.
 */
void CAbstractStreaming::RequestModel(id_t id, uint32_t flags)
{
    injector::cstd<void(int, int)>::call<0x4087E0>(id, flags);  // CStreaming::RequestModel
}

/*
 *  CAbstractStreaming::RemoveModel
 *      Removes a resource from the streaming, essentially unloading it
 *      Flags are unknown (and optional) so leave it as default.
 */
void CAbstractStreaming::RemoveModel(id_t id)
{
    injector::cstd<void(int)>::call<0x4089A0>(id);  // CStreaming::RemoveModel
}

/*
 *  CAbstractStreaming::LoadAllRequestedModels
 *      Free ups the streaming bus by loading everything previosly requested
 */
void CAbstractStreaming::LoadAllRequestedModels()
{
    plugin_ptr->Log("Loading requested resources...");
    injector::cstd<void()>::call<0x5619D0>();       // CTimer::Suspend
    injector::cstd<void(int)>::call<0x40EA10>(0);   // CStreaming::LoadAllRequestedModels
    injector::cstd<void(int)>::call<0x40EA10>(0);   // CStreaming::LoadAllRequestedModels
    injector::cstd<void()>::call<0x561A00>();       // CTimer::Resume
}

/*
 *  CAbstractStreaming::FlushChannels
 *      Flushes the streaming channels loading anything on the bus
 */
void CAbstractStreaming::FlushChannels()
{
    plugin_ptr->Log("Flushing streaming channels...");
    injector::cstd<void()>::call<0x5619D0>();       // CTimer::Suspend
    injector::cstd<void(int)>::call<0x40E460>(0);   // CStreaming::FlushChannels
    injector::cstd<void()>::call<0x561A00>();       // CTimer::Resume
}

/*
 *  CAbstractStreaming::RemoveUnusedResources
 *      Free ups the streaming by removing any unused resource
 */
void CAbstractStreaming::RemoveUnusedResources()
{
    plugin_ptr->Log("Removing unused resources...");
    injector::cstd<void()>::call<0x5619D0>();               // CTimer::Suspend
    injector::cstd<void()>::call<0x40CF80>();               // CStreaming::RemoveAllUnusedModels
    injector::cstd<char(uint32_t)>::call<0x40CFD0>(0x20);   // CStreaming::RemoveLeastUsedModel
    injector::cstd<void()>::call<0x561A00>();               // CTimer::Resume
}

/*
 *  CAbstractStreaming::RegisterModelIndex
 *      Registers the existence of an index assigned to a specific filename.
 */
void CAbstractStreaming::RegisterModelIndex(const char* filename, id_t index)
{
    this->indices[mhash(filename)] = index;
}

/*
 *  CAbstractStreaming::RegisterClothingItem
 *      Registers the existence of a clothing item
 */
void CAbstractStreaming::RegisterClothingItem(const char* filename, int index)
{
    this->clothes_map[mhash(filename)] = index;
}

/*
 *  CAbstractStreaming::RegisterStockEntry
 *      Registers the stock/default/original cd directory data of an index, important so it can be restored later when necessary.
 */
void CAbstractStreaming::RegisterStockEntry(const char* filename, DirectoryInfo& entry, id_t index, int img_id)
{
    // Please note entry here is incomplete because the game null terminated the string before the extension '.', so use filename
    this->cd_dir[index] = CdDirectoryItem(filename, entry, img_id);
}






/*
 *  CAbstractStreaming::InstallFile
 *      Installs a model, clothing or any streamable file, refreshing them after the process
 */
bool CAbstractStreaming::InstallFile(const modloader::file& file)
{
    // If the streaming hasn't initialized we cannot assume much things about the streaming
    // One thing to keep in mind is that in principle stuff should load in alpha order (for streamed scenes etc)
    if(!this->bHasInitializedStreaming)
    {
        // Just push it to this list and it will get loaded when the streaming initializes
        // At this point we don't know if this is a clothing item or an model, for that reason "raw"
        // The initializer will take care of filtering clothes and models from the list
        this->raw_models[file.filename()] = &file;
        return true;
    }
    else
    {
        // We cannot do much at this point, too many calls may come, repeated calls, uninstalls, well, many things will still happen
        // so we'll delay the actual install to the next frame, put everything on an import list
        this->BeginUpdate();

        if(IsNonStreamed(&file))
            return false;
        else if(!IsClothes(&file))
        {
            this->to_import[file.hash] = &file;
            return true;
        }
        else
        {
            auto size_blocks = GetSizeInBlocks(file.size);
            this->newcloth_blocks   = size_blocks > newcloth_blocks? size_blocks : newcloth_blocks;
            this->to_rebuild_player = true;
            return this->ImportCloth(&file);
        }
    }
    return false;
}

/*
 *  CAbstractStreaming::UninstallFile
 *      Uninstalls a specific file
 */
bool CAbstractStreaming::UninstallFile(const modloader::file& file)
{
    // Ahhh, see the comments at InstallFile.....
    if(!this->bHasInitializedStreaming)
    {
        if(IsNonStreamed(&file))
            return false;

        // Streaming hasn't initialized, just remove it from our raw list
        raw_models.erase(file.filename());
        return true;
    }
    else
    {
        this->BeginUpdate();

        if(IsNonStreamed(&file))
            return false;
        else if(!IsClothes(&file))
        {
            // Remove special model (if it is a special model)
            erase_from_map(special, &file);     

            // Mark the specified file [hash] to be vanished
            this->to_import[file.hash] = nullptr;
            return true;
        }
        else
        {
            this->to_rebuild_player = true;
            return this->UnimportCloth(&file);
        }
    }
    return false;
}


/*
 *  CAbstractStreaming::ReinstallFile
 *      Does the same as InstallFile
 */
bool CAbstractStreaming::ReinstallFile(const modloader::file& file)
{
    // Reinstalling works the same way as installing
    // Do success even if InstallFile fails, the failure might be related to already having a file installed
    InstallFile(file);
    return true;            
}


/*
 *  CAbstractStreaming::Update
 *      Updates the abstract streaming after a serie of install/uninstall
 */
void CAbstractStreaming::Update()
{
    if(this->IsUpdating())
    {
        // Refresh necessary files
        this->ProcessRefreshes();
        this->EndUpdate();
    }
}




/*
 *  CAbstractStreaming::ImportModels
 *      Imports the files in the list into the abstract streaming
 */
void CAbstractStreaming::ImportModels(ref_list<const modloader::file*> files)
{
    LoadAbstractCdDirectory(files);
}

/*
 *  CAbstractStreaming::UnimportModel
 *      Removes the imported index from the abstract streaming
 */
void CAbstractStreaming::UnimportModel(id_t index)
{
    // Remove special model related to this index if possible
    auto it = imports.find(index);
    if(it != imports.end()) erase_from_map(special, it->second.file);

    // Restore model information to stock and erase it from the import list
    this->RestoreInfoForModel(index);
    this->QuickUnimport(index);
}


/*
 *  CAbstractStreaming::QuickImport
 *      Quickly import into the specified index the specified file with the specified flags
 *      This is a raw import technique and should be used ONLY when necessary
 *      It does not perform any kind of checking such as if the streaming buffer size is enought
 */
void CAbstractStreaming::QuickImport(id_t index, const modloader::file* file, bool isSpecialModel, bool isCloth)
{
    plugin_ptr->Log("Importing model file for index %d at \"%s\"", index, file->filepath());

    // Add import into the import table
    auto& imp = imports[index];
    imp.file = file;
    imp.isFallingBack = false;
    imp.isSpecialModel = isSpecialModel;
    imp.isClothes = isCloth;

    // Register the existence of such a model and setup info for it
    if(!isCloth) this->RegisterModelIndex(file->filename(), index);
    this->SetInfoForModel(index, 0, GetSizeInBlocks(file->size));
}

/*
 *  CAbstractStreaming::QuickUnimport
 *      Quickly unimport the specified index
 *      Analogue to QuickImport. This version of unimporting do not mess with the info for model structure.
 */
void CAbstractStreaming::QuickUnimport(id_t index)
{
    plugin_ptr->Log("Removing imported model file at index %d", index);
    this->imports.erase(index);
}






/*
 *  CAbstractStreaming::IsClothes
 *      Checks if a specific file is a clothing item
 */
bool CAbstractStreaming::IsClothes(const modloader::file* file)
{
    if(gvm.IsSA())  // Only San Andreas has clothing items
    {
        if(file->behaviour & is_fcloth_mask)    
            return true;    // Forced clothes
        else if(!this->indices.count(file->hash) && this->clothes_map.count(file->hash))
            return true;    // Not present in standard indices but in clothes map
        else if(!strcmp(file->filename(), "coach.dff"))
        {
            // Coach is both a clothing item and a vehicle, what is up with this file?
            // If the number of Clump sections on the RwStream is greater than 1, it's a clothing item.
            if(auto f = fopen(file->fullpath().c_str(), "rb"))
            {
                struct {
                    uint32_t id, size, version;
                } section;
                int nclumps = 0;

                while(fread(&section, sizeof(section), 1, f) == 1)
                {
                    if(section.id == 0x0010)    // Clump
                    {
                        ++nclumps;
                        if(!!fseek(f, section.size, SEEK_CUR))
                            break;
                    }
                    else break;
                }

                fclose(f);

                // If more than one clump in the RwStream, it's a clothing item
                bool is_cloth = (nclumps > 1);
                if(is_cloth) plugin_ptr->Log("Warning: Clothing coach item outside a player.img directory: %s", file->filepath());
                return is_cloth;
            }
        }
    }
    return false;
}

/*
 *  CAbstractStreaming::ImportCloth
 *      Imports the specified clothing item
 *      !!!!! Notice that importing a clothing item doesn't contribute to the streaming buffer, please use
 *      'StreamingBufferUpdater' manually to do so !!!!
 */
bool CAbstractStreaming::ImportCloth(const modloader::file* file)
{
    if(gvm.IsSA())
    {
        auto clothesDirectory = (CDirectorySA*)(::clothesDirectory);

        if(auto entry = FindClothEntry(file->hash))
        {
            // Every cloth import should go through this code path....
            // The else condition calls this function again so it goes by this path

            // Allow import only if we don't have any clothing like that installed
            return this->clothes.emplace(entry->m_dwFileOffset, file).second;
        }
        else if(true)
        {
            // We need to add a new entry into the clothes directory...
            DirectoryInfo entry;
            plugin_ptr->Log("Adding new item to clothing directory \"%s\"", file->filename());
            this->RegisterClothingItem(file->filename(), clothesDirectory->m_dwCount);              // Tell us about it
            FillDirectoryEntry(entry, file->filename(), TakeClothSparseOffset(), file->size);
            injector::thiscall<void(CDirectorySA*, DirectoryInfo*)>::call<0x532310>(clothesDirectory, &entry);  // CDirectory::AddItem
            sparse_dir_entries.emplace_back(entry);

            // Try again, now it'll work
            return this->ImportCloth(file); 
        }
    }
    return false;
}

/*
 *  CAbstractStreaming::UnimportCloth
 *      Unimports the specified clothing item
 */
bool CAbstractStreaming::UnimportCloth(const modloader::file* file)
{
    // Right, since the index is a file offset we need to iterate on the map until we find this file
    for(auto it = this->clothes.begin(); it != clothes.end(); ++it)
    {
        if(it->second == file)
        {
            // We cannot unimport added clothing items during runtime!!!
            if(this->IsClothSparseOffset(it->first) == false)
            {
                // Fine, let's unimport
                this->clothes.erase(it);
                return true;
            }
            return false;
        }
    }
    return false;
}
