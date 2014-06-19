add_library(std.img SHARED img.cpp streaming.cpp backend.${ASM_EXT} $<TARGET_OBJECTS:addr>)
install(TARGETS std.img RUNTIME DESTINATION "./modloader/.data/plugins/gta3")

