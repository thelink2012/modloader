set(SOURCE_FILES
     loader.cpp loader.hpp plugins.cpp folder.cpp modinfo.cpp
     exception.cpp logging.cpp

     extras/gta3/menu.cpp
     extras/gta3/menu.hpp
     extras/gta3/MenuExtender/MenuExtender.cpp
     extras/gta3/MenuExtender/MenuExtender.h

     "${CMAKE_SOURCE_DIR}/include/modloader/modloader.h"
     "${CMAKE_SOURCE_DIR}/include/modloader/modloader.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/gta3/gta3.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/container.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/file.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/fxt.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/hash.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/ini.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/injector.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/detour.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/parser.hpp"
     "${CMAKE_SOURCE_DIR}/include/modloader/util/path.hpp"
)

add_library(modloader SHARED ${SOURCE_FILES} $<TARGET_OBJECTS:addr>)
target_link_libraries(modloader shlwapi dbghelp)    # (shlwapi) PathMatchSpec, (dbghelp) for stack walking and symbol finding
set_target_properties(modloader PROPERTIES SUFFIX ".asi")
install(TARGETS modloader RUNTIME DESTINATION "./")

if(MSVC)
    install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/modloader.pdb" DESTINATION "./" CONFIGURATIONS Debug)
endif(MSVC)
