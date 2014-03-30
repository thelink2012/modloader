add_library(modloader SHARED main.cpp exception.cpp $<TARGET_OBJECTS:addr>)
set_target_properties(modloader PROPERTIES SUFFIX ".asi")
install(TARGETS modloader RUNTIME DESTINATION "./")
