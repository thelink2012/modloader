project(modloader)
cmake_minimum_required(VERSION 2.8)

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

# Find all plugins readme files
file(GLOB plugins_doc "${DOC_DIRECTORY}/plugins/*.txt")

# Find all readme files
file(GLOB readme_doc  "${DOC_DIRECTORY}/readme/*.txt")

# Install plugins documentation into modloader plugins folder
install(FILES ${plugins_doc} DESTINATION "./modloader/.data/plugins")

# Install readme files, README.md, LICENSE and CHANGELOG into modloader/.data
install(FILES "${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/CHANGELOG" DESTINATION "./modloader/.data")
install(FILES ${readme_doc}  DESTINATION "./modloader/.data")

# Put readme files on base, so the user can see it
install(FILES ${readme_doc}  DESTINATION "./")  

# Install config files
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Don't copy configs in Debug
else()
    install(FILES "${CONFIGS_DIR}/modloader.ini" DESTINATION "./modloader")
    install(FILES "${CONFIGS_DIR}/plugins.ini" DESTINATION "./modloader/.data/plugins")
endif()



