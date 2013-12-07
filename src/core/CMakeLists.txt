add_library(modloader SHARED main.cpp)
set_target_properties(modloader PROPERTIES SUFFIX ".asi")
install(TARGETS modloader RUNTIME DESTINATION "./")
