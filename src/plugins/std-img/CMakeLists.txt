add_library(std-img SHARED img.cpp hooks.cpp hooks.${ASM_EXT} streaming.cpp)
install(TARGETS std-img RUNTIME DESTINATION "./modloader/.data/plugins")

