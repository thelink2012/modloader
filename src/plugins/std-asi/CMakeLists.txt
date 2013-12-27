add_library(std-asi SHARED asi.cpp)
install(TARGETS std-asi RUNTIME DESTINATION "./modloader/.data/plugins")
