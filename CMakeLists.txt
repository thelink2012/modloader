project(modloader)
cmake_minimum_required(VERSION 2.8)

set(DOC_DIRECTORY "${CMAKE_SOURCE_DIR}/doc")

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

# install commmand
set(CMAKE_INSTALL_PREFIX "")
file(GLOB asi "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/*.asi")
file(GLOB plugins "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/*.dll")
file(GLOB plugins_doc "${DOC_DIRECTORY}/plugins/*.txt")
install(FILES ${plugins_doc} DESTINATION "./modloader/.data/plugins")
install(FILES "${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/CHANGELOG" DESTINATION "./modloader")
