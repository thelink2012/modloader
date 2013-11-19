add_library(std-fx SHARED fx.cpp)
install(TARGETS std-fx RUNTIME DESTINATION "./modloader/.plugins")
