add_library(std-cleo SHARED cleo.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-cleo RUNTIME DESTINATION "./modloader/.data/plugins")
