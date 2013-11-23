add_library(std-cleo SHARED cleo.cpp)
install(TARGETS std-cleo RUNTIME DESTINATION "./modloader/.data/plugins")
