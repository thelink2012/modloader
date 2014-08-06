project(modloader)
cmake_minimum_required(VERSION 2.8)

# When no build type specified, use Release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
    message("Using default build type that is Release")
endif()


set(DOC_DIRECTORY "${CMAKE_SOURCE_DIR}/doc")
set(CONFIGS_DIR "${DOC_DIRECTORY}/config")

# Output directories
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(MAKE_SHARED_MODULE_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Ok, run source directory
include_directories("${CMAKE_SOURCE_DIR}/include")
add_subdirectory(src)



# ----------------------------> Install
set(CMAKE_INSTALL_PREFIX "")
set(plugins_doc "${DOC_DIRECTORY}/plugins/")
set(readme_doc "${DOC_DIRECTORY}/readme/")
set(locale_doc "${DOC_DIRECTORY}/text/")

# Install plugins documentation into modloader plugins folder
install(DIRECTORY ${plugins_doc} DESTINATION "./modloader/.data/plugins")

# Install readme files, README.md, LICENSE and CHANGELOG into modloader/.data
install(DIRECTORY ${readme_doc}  DESTINATION "./modloader/.data")
install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/CHANGELOG.md" "${DOC_DIRECTORY}/Command Line Arguments.md" DESTINATION "./modloader/.data")

# Install locale data
install(DIRECTORY ${locale_doc}  DESTINATION "./modloader/.data/text")

# Install config files
install(FILES "${CONFIGS_DIR}/config.ini.0" DESTINATION "./modloader/.data")
install(FILES "${CONFIGS_DIR}/modloader.ini.0" DESTINATION "./modloader/.data")
install(FILES "${CONFIGS_DIR}/plugins.ini.0" DESTINATION "./modloader/.data/plugins")
