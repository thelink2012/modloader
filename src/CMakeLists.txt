# The current directory shall be used for #include <file>
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DINJECTOR_GVM_HAS_TRANSLATOR -DINJECTOR_GVM_PLUGIN_NAME="Mod Loader Plugin")
add_definitions(-DNOMINMAX)

# Visual Studio specific stuff
if(MSVC)
    set(ASM_EXT "c")	 # Use C with declspec(naked) instead

    # We don't want Buffer Security Check
    set(CMAKE_CXX_FLAGS_RELEASE "/GS- ${CMAKE_CXX_FLAGS_RELEASE}")
    
   # Statically link, we don't want to depend on the dll (and we don't want RTTI)
    foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                     CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
    
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif(MSVC)

# MiNGW specific stuff
if(MINGW)
    # Enable ASM files
    enable_language(C CXX ASM)
    set(ASM_EXT "s")  # GAS Assembly extension

    # Compiling C++11 source (with GNU and Microsoft extensions)
    set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
    
    # Force static runtime
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -Wl,--export-all-symbols") # Export all symbols for better user logging

    # Nothing here uses exceptions neither RTTI, MinGW sjlj is kinda of slow
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
    
endif(MINGW)

#
include_directories("structs/gta3")
include_directories("util")

#
add_subdirectory(address_translator)
add_subdirectory(core)
add_subdirectory(plugins)


