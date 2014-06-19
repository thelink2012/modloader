add_library(std.text SHARED text.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std.text RUNTIME DESTINATION "./modloader/.data/plugins/gta3")
