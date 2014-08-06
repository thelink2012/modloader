set(SOURCE_FILES
    asi.cpp asi.h CsInfo.cpp ModuleInfo.cpp
    args_translator/xtranslator.hpp
    args_translator/xtranslator_path.hpp
    args_translator/translator.hpp
    args_translator/translator_basic.hpp
    args_translator/translator_cdecl.hpp
    args_translator/translator_stdcall.hpp
    args_translator/hacks/Bass.hpp
    args_translator/hacks/FindCleoScripts.hpp
    args_translator/hacks/RyosukeSetModule.hpp
)

add_library(std.asi SHARED ${SOURCE_FILES} $<TARGET_OBJECTS:addr>)
install(TARGETS std.asi RUNTIME DESTINATION "./modloader/.data/plugins/gta3")
