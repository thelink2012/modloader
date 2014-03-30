add_library(std-fx SHARED fx.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-fx RUNTIME DESTINATION "./modloader/.data/plugins")
