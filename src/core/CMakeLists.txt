set(SOURCE_FILES
     loader.cpp loader.hpp plugins.cpp folder.cpp modinfo.cpp
     exception.cpp logging.cpp

     extras/gta3/menu.cpp
     extras/gta3/menu.hpp
     extras/gta3/MenuExtender/MenuExtender.cpp
     extras/gta3/MenuExtender/MenuExtender.h
)

add_library(modloader SHARED ${SOURCE_FILES} $<TARGET_OBJECTS:addr>)
target_link_libraries(modloader shlwapi dbghelp)    # (shlwapi) PathMatchSpec, (dbghelp) for stack walking and symbol finding
set_target_properties(modloader PROPERTIES SUFFIX ".asi")
install(TARGETS modloader RUNTIME DESTINATION "./")

if(MSVC)
    install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/modloader.pdb" DESTINATION "./" CONFIGURATIONS Debug)
endif(MSVC)
