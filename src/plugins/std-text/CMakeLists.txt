add_library(std-text SHARED text.cpp)
install(TARGETS std-text RUNTIME DESTINATION "./modloader/.data/plugins")
