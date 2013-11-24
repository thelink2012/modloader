add_library(std-sprites SHARED sprites.cpp)
install(TARGETS std-sprites RUNTIME DESTINATION "./modloader/.data/plugins")
