add_library(std-scm SHARED scm.cpp $<TARGET_OBJECTS:addr>)
install(TARGETS std-scm RUNTIME DESTINATION "./modloader/.data/plugins")
