#pragma once
#include <modloader/modloader.h>
#include <cstdint>

typedef struct {

    uint32_t size;

    int* p_gGameState;

    void (*PlayMovieInWindow)(int, const char*);

} modloader_re3_addr_table_t;

typedef struct {

    uint32_t size;

    void (*PlayMovieInWindow_Logo)(int, const char*);
    void (*PlayMovieInWindow_GTAtitles)(int, const char*);

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