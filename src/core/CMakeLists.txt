add_library(modloader SHARED loader.cpp exception.cpp logging.cpp plugins.cpp folder.cpp modinfo.cpp $<TARGET_OBJECTS:addr>)
target_link_libraries(modloader shlwapi)    # for PathMatchSpec
set_target_properties(modloader PROPERTIES SUFFIX ".asi")
install(TARGETS modloader RUNTIME DESTINATION "./")
