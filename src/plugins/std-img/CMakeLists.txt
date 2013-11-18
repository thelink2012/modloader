add_library(std-img SHARED img.cpp hooks.cpp hooks.s streaming.cpp)
install(TARGETS std-img RUNTIME DESTINATION "./modloader/.plugins")

