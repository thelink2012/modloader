add_library(std-tracks SHARED tracks.cpp)
install(TARGETS std-tracks RUNTIME DESTINATION "./modloader/.data/plugins")

