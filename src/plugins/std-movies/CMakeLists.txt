add_library(std-movies SHARED movies.cpp)
install(TARGETS std-movies RUNTIME DESTINATION "./modloader/.plugins")

