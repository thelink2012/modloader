set(SOURCE_FILES
main.cpp
data.cpp
streaming.cpp
streaming.hpp
directory.cpp
refresh.cpp
backend.cpp
asm/backend.${ASM_EXT}
)

add_library(std.stream SHARED ${SOURCE_FILES} $<TARGET_OBJECTS:addr>)
install(TARGETS std.stream RUNTIME DESTINATION "./modloader/.data/plugins/gta3")

