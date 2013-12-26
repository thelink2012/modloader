add_library(std-sala SHARED sala.cpp sacky.cpp)
install(TARGETS std-sala RUNTIME DESTINATION "./modloader/.data/plugins")

