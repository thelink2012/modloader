--[[
    Mod Loader Build Script
    Use 'premake5 --help' for help
--]]



--[[
    Options and Actions
--]]

newoption {
    trigger     = "outdir",
    value       = "path",
    description = "Output directory for the build files"
}
if not _OPTIONS["outdir"] then
    _OPTIONS["outdir"] = "build"
end

newoption {
    trigger     = "idir",
    value       = "path",
    description = "Post-build install directory"
}

newaction {
    trigger     = "clean",
    description = "Cleans the binary and build files on the directory (bin/, build_temp/)",
    execute     = function()
        os.rmdir("bin")
        os.rmdir("build_temp")
        os.rmdir("release")
    end
}

newaction {
    trigger     = "install",
    description = "Installs a previosly built Mod Loader into the directory specified in the 2nd argument",
    execute     = function()
        local dest = _ARGS[1]
        if dest == nil then
            print("Missing 2nd argument which should be the install directory.\nAborting.")
        else
            dest = makeabsolute(dest)
            print("Installing into \"" .. dest .. "\" directory")
            for i, cmd in ipairs(installcommands(dest)) do
                --print(os.outputof(cmd))
                os.execute(cmd)
            end
        end
    end
}




--[[
    Install Functionality
--]]
install_files = {}
cmd_copyfile = os.is("windows") and { "xcopy", "/f /y /i" }    or { "cp", "-v" }
cmd_copydir  = os.is("windows") and { "xcopy", "/e /f /y /i" } or { "cp", "-vr" }

-- Gets the install command for the specified file
function installcommand(file, destdir)

    if file.isdir == nil then
        file.isdir = not os.isfile(file.source)
    end

    local cmd = string.format(path.translate([[%s "%s/%s" "%s/%s%s" %s]]),
                    file.isdir and cmd_copydir[1] or cmd_copyfile[1],
                    path.translate(_MAIN_SCRIPT_DIR), path.translate(file.source),
                    path.translate(destdir), path.translate(file.destination), path.translate(file.isdir and "" or "/*"),
                    file.isdir and cmd_copydir[2] or cmd_copyfile[2])

    return cmd
end

-- Gets all install commands based on all files sent to 'addinstall'
function installcommands(destdir)
    local cmds = {}

    for i, file in ipairs(install_files) do
        table.insert(cmds, installcommand(file, destdir))
    end

    return cmds
end

-- Adds a file to be installed
function addinstall(file)
    table.insert(install_files, file)
    if _OPTIONS["idir"] then
        postbuildcommands { installcommand(file, makeabsolute(_OPTIONS["idir"])) }
    end
end

function makeabsolute(pathx)
    return path.isabsolute(pathx) and pathx or (_MAIN_SCRIPT_DIR .. '/' .. pathx)
end



--[[
    Solution Setup Utilities
--]]
asm_extension = (_ACTION == "gmake" and "s" or "cc")    -- (note: dont use .c as the extension for msvc, breaks pch)

function binarydir(dir)
    targetdir("bin/" .. dir)
    implibdir("bin/" .. dir)
end

function setupfiles(dir)
    files {
        dir .. "/**.cpp",
        dir .. "/**.h",
        dir .. "/**.hpp",
        dir .. "/**." .. asm_extension 
    }
end

function pchsetup(pchdir)
    pchheader "stdinc.hpp"
    pchsource "src/shared/stdinc/stdinc.cpp"
    files { "src/shared/stdinc/stdinc.cpp" }
    includedirs { pchdir }
end

function addplugin(name)

    local directory = ("src/plugins/gta3/" .. name .. "/")
    local has_pch   = os.isfile(directory .. "/stdinc.hpp")
    local pch_dir   = has_pch and directory or "src/shared/stdinc/gta3/"

    project(name)
        language "C++"
        kind "SharedLib"

        binarydir "plugins/gta3"
        addinstall({
            isdir = false,
            source = "bin/plugins/gta3/" .. name .. ".dll",
            destination = "modloader/.data/plugins/gta3"
        })

        includedirs {
            "src/shared/game/gta3"
        }

        links { "addr" }
        dependson { "modloader" }
        setupfiles(directory)
        pchsetup(pch_dir)
    
end

function dummyproject()

    kind "Makefile"
    language "C++"
    flags { "NoPCH" }

    -- Dummy cpp file for Premake's generated none  project (bug workaround)
    configuration "gmake"
        kind "StaticLib"
        files { "src/shared/dummy.cpp" }
        
    configuration {}
end

--[[
    The Solution
--]]
solution "modloader"

    startproject "build_gta3"

    configurations { "Release", "Debug" }

    location( _OPTIONS["outdir"] )
    targetprefix "" -- no 'lib' prefix on gcc
    targetdir "bin"
    implibdir "bin"

    flags {
        "StaticRuntime",
        "Symbols",          -- Produce symbols whenever possible for logging purposes
        "NoImportLib",      -- Mod Loader itself and it's plugins are dlls which exports some funcs but a implib isn't required
        --"NoRTTI", (std.data uses it now on handling.cpp)
        "NoBufferSecurityCheck"
    }

    defines {
        "INJECTOR_GVM_HAS_TRANSLATOR",
        'INJECTOR_GVM_PLUGIN_NAME="\\"Mod Loader Plugin\\""'    -- (additional quotes needed for gmake)
    }

    defines {
        "NOMINMAX",
        "_CRT_SECURE_NO_WARNINGS",
        "_SCL_SECURE_NO_WARNINGS"
    }

    includedirs {
        "include",
        "deps/cereal/include",
        "deps/injector/include",
        "deps/ini_parser/include",
        "deps/fxt_parser/include",
        "deps/datalib/include",
        "deps/type_wrapper/include",
        "deps/boost",
        "deps/tinympl",
        "src/shared",
    }

    configuration "Debug*"
        flags { "Symbols" }
        
    configuration "Release*"
        defines { "NDEBUG" }
        optimize "Speed"

    configuration "gmake"
        buildoptions { "-std=gnu++14", "-Wno-deprecated" }
    configuration "vs*"
        buildoptions { "/arch:IA32" }   -- disable the use of SSE/SSE2 instructions (old game, old computers)
        buildoptions { "/Zm150" }       -- more precompiled header memory (for gta3.std.data)

    project "docs"
        dummyproject()
        files { "doc/**" }
        dependson { "modloader" }
        addinstall { source = "LICENSE",                        destination = "modloader/.data"         }
        addinstall { source = "doc/licenses",                   destination = "modloader/.data/licenses"}
        addinstall { source = "doc/text",                       destination = "modloader/.data/text"    }
        addinstall { source = "doc/readme",                     destination = "modloader/.data"         }
        addinstall { source = "doc/plugins",                    destination = "modloader/.data/plugins" }
        addinstall { source = "doc/config/config.ini.0",        destination = "modloader/.data"         }
        addinstall { source = "doc/config/modloader.ini.0",     destination = "modloader/.data"         }
        addinstall { source = "doc/config/plugins.ini.0",       destination = "modloader/.data"         }
        addinstall { source = "doc/CHANGELOG.md",               destination = "modloader/.data"         }
        addinstall { source = "doc/Command Line Arguments.md",  destination = "modloader/.data"         }
        addinstall { source = "doc/Profiles.md",                destination = "modloader/.data"         }

    project "addr"
        language "C++"
        kind "StaticLib"
        flags { "NoPCH" }
        setupfiles "src/translator"

    project "modloader"
        language "C++"
        kind "SharedLib"
        targetname "modloader"
        targetextension ".asi"
        binarydir ""
        addinstall( { isdir = false, source = "bin/modloader.asi", destination = "./" } )
        links { "addr", "shlwapi", "dbghelp" }
        setupfiles "include"
        setupfiles "src/core"
        pchsetup "src/core"

    project "shared"
        dummyproject()
        setupfiles "src/shared"
        configuration { "gmake" }
            includedirs { "src/shared/stdinc" } -- gmake compatibility since it'll compile the dummyproject


    local gta3_plugins = {  -- ordered by time taken to compile
        "std.movies",
        "std.scm",
        "std.sprites",
        "std.fx",
        "std.text",
        "std.tracks",
        "std.bank",
        "std.stream",
        "std.asi",
        "std.data"
    }

    project "build_gta3"
        dummyproject()
        dependson("modloader")
        dependson(gta3_plugins)

        for i, name in ipairs(gta3_plugins) do
            addplugin(name)
        end


