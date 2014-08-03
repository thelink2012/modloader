add_library(std.data SHARED main.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std.data RUNTIME DESTINATION "./modloader/.data/plugins/gta3")
