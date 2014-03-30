add_library(std-sprites SHARED sprites.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-sprites RUNTIME DESTINATION "./modloader/.data/plugins")
