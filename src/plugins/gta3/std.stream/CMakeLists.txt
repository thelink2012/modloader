add_library(std.stream SHARED plugin.cpp streaming.cpp streaming.hpp "asm/backend.${ASM_EXT}" $<TARGET_OBJECTS:addr>)
install(TARGETS std.stream RUNTIME DESTINATION "./modloader/.data/plugins/gta3")

