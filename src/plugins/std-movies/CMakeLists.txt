add_library(std-movies SHARED movies.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-movies RUNTIME DESTINATION "./modloader/.data/plugins")

