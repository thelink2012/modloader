add_library(modloader SHARED main.cpp LogException.cpp)
set_target_properties(modloader PROPERTIES SUFFIX ".asi")
install(TARGETS modloader RUNTIME DESTINATION "./")
