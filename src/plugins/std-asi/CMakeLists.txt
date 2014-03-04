add_library(std-asi SHARED asi.cpp CsInfo.cpp ModuleInfo.cpp)
install(TARGETS std-asi RUNTIME DESTINATION "./modloader/.data/plugins")
