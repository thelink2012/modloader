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

    # Compiling C++11 source (with GNU and Microsoft extensions)
    set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
    
    # Force static runtime (I guess those lines are real dirty CMake lines...)
    # As you may know Windows DLLs and GNU standard library do not work very well together since the GNU librares have different exception models within the same DLL name, that's real bad.
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static")

    # Nothing here uses exceptions neither RTTI
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
    
endif(MINGW)



#
include_directories("Game Structs")

#
add_subdirectory(core)
add_subdirectory(plugins)
