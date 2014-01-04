add_library(std-img SHARED img.cpp streaming.cpp backend.${ASM_EXT})
install(TARGETS std-img RUNTIME DESTINATION "./modloader/.data/plugins")

