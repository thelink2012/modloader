#
#   San Andreas modloader
#       CMake file script
#

# The current directory shall be used for #include <file>
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Enable ASM files
enable_language(C CXX ASM)


# Visual Studio specific stuff
if(MSVC)
    set(ASM_EXT "asm")  # MASM Assembly extension

    # We don't want to deal with Buffer Security Check under "Release" build.
    set(CMAKE_CXX_FLAGS_RELEASE "/GS- ${CMAKE_CXX_FLAGS_RELEASE}")
endif(MSVC)

# MiNGW specific stuff
if(MINGW)
    set(ASM_EXT "s")  # GAS Assembly extension

    # Strip debug symbols on release (not working, must do manually)
    set(CMAKE_CXX_FLAGS_RELEASE "-s ${CMAKE_CXX_FLAGS_RELEASE}")
    
    # Compiling C++11 source (with GNU and Microsoft extensions)
    set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
    # Static runtime (not working)
    set(CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")
    # Nothing here uses exceptions (RTTI too, who uses RTTI?)
    set(CMAKE_CXX_FLAGS "-fno-exceptions -fno-rtti ${CMAKE_CXX_FLAGS}")
endif(MINGW)



#
include_directories("Game Structs")

#
add_subdirectory(core)
add_subdirectory(plugins)
