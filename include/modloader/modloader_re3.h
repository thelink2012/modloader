#pragma once
#include <modloader/modloader.h>
#include <cstdint>

typedef struct {

    uint32_t size;

    int* p_gGameState;

    void (*PlayMovieInWindow)(int, const char*);

    void* ms_aInfoForModel;
    uint32_t uMaxResources;
    DWORD* p_gCdStreamFlags;
    HANDLE* p_gCdStreamSema;
    void* p_gChannelRequestQ;
    void** p_gpReadInfo;
    BOOL* p_gbCdStreamOverlapped;
    BOOL* p_gbCdStreamAsync;
    void** p_ms_pStreamingBuffer;
    uint32_t* p_ms_streamingBufferSize;
    bool (*CDirectory__FindItem4)(void*, const char*, uint32_t*, uint32_t*);
    void (*CStreaming__LoadCdDirectory0)();
    void (*CStreaming__LoadCdDirectory2)(const char* filename, int cd_index);
    int32_t(*CdStreamRead)(int32_t channel, void* buffer, uint32_t offset, uint32_t size);

    void (*LoadCdDirectoryUsingCallbacks)(void* pUserData, int n, bool (*ReadEntry)(void*, void*, uint32_t),
        bool (*RegisterEntry)(void*, void*, bool), void (*RegisterSpecialEntry)(void*, void*));


} modloader_re3_addr_table_t;

typedef struct {

    uint32_t size;

    void (*PlayMovieInWindow_Logo)(int, const char*);
    void (*PlayMovieInWindow_GTAtitles)(int, const char*);

    void (*CdStreamThread)();
    void (*LoadCdDirectory0)();
    void (*RegisterNextModelRead)(uint32_t);
    HANDLE (*AcquireNextModelFileHandle)();
    void (*FetchCdDirectory)(const char*, int);
    int32_t (*CdStreamRead)(int32_t channel, void* buffer, uint32_t offset, uint32_t size);

    void(*OnRequestSpecialModel)(uint32_t model_id, const char* model_name, uint32_t pos, uint32_t size);

} modloader_re3_callback_table_t;

typedef struct modloader_re3_t {

    uint32_t size;
    uint32_t re3_version;

    modloader_t* modloader;
    modloader_re3_addr_table_t* re3_addr_table;
    modloader_re3_callback_table_t* callback_table;

    void (*Tick)(struct modloader_re3_t*);
    void (*Shutdown)(struct modloader_re3_t*);

    void* movies_plugin_ptr;

} modloader_re3_t;

typedef int (*modloader_fInitFromRE3)(modloader_re3_t*);