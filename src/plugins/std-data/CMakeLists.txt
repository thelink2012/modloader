add_library(std-data SHARED data.cpp ipl.cpp handling.cpp)
install(TARGETS std-data RUNTIME DESTINATION "./modloader/.data/plugins")
