add_library(std.data SHARED data.cpp ipl.cpp handling.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std.data RUNTIME DESTINATION "./modloader/.data/plugins/gta3")
