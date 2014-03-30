add_library(std-tracks SHARED tracks.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-tracks RUNTIME DESTINATION "./modloader/.data/plugins")

