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
                print(os.outputof(cmd))
            end
        end
    end
}




--[[
    Install Functionality
--]]
install_files = {}
cmd_copyfile = os.is("windows") and { "xcopy", "/f /y" }       or { "cp", "-v" }
cmd_copydir  = os.is("windows") and { "xcopy", "/e /f /y /i" } or { "cp", "-vr" }

-- Gets the install command for the specified file
function installcommand(file, destdir)

    if file.isdir == nil then
        file.isdir = not os.isfile(file.source)
    end

    local cmd = string.format(path.translate([[%s "%s/%s" "%s/%s" %s]]),
                    file.isdir and cmd_copydir[1] or cmd_copyfile[1],
                    path.translate(_MAIN_SCRIPT_DIR), path.translate(file.source),
                    path.translate(destdir), path.translate(file.destination),
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
    return path.isabsolute(pathx) and p or (_MAIN_SCRIPT_DIR .. '/' .. pathx)
end



--[[
    Solution Setup Utilities
--]]
asm_extension = (_ACTION == "gmake" and "s" or "c")

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

function addplugin(name)

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
            "src/structs/gta3"
        }

        links { "addr" }
        setupfiles("src/plugins/gta3/" .. name)
        
end

--[[
    The Solution
--]]
solution "modloader"

    configurations { "Release", "Debug" }

    location( _OPTIONS["outdir"] )
    targetprefix "" -- no 'lib' prefix on gcc
    targetdir "bin"
    implibdir "bin"

    flags {
        "StaticRuntime",
        "Symbols",          -- Produce symbols whenever possible for logging purposes
        "NoImportLib",      -- Mod Loader itself and it's plugins are dlls which exports some funcs but a implib isn't required
        "NoRTTI",
        "NoBufferSecurityCheck",
        "NoPCH"
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
        "deps/injector/include",
        "deps/ini_parser/include",
        "src/util",
        "src/traits"
    }

    configuration "Debug*"
        flags { "Symbols" }
        
    configuration "Release*"
        defines { "NDEBUG" }
        optimize "Speed"

    configuration "gmake"
        buildoptions { "-std=c++11" }


    project "addr"
        language "C++"
        kind "StaticLib"
        setupfiles "src/address_translator"

    project "docs"
        kind "None"
        files { "doc/**" }
        addinstall { source = "LICENSE",                        destination = "modloader/.data"         }
        addinstall { source = "doc/text",                       destination = "modloader/.data/text"    }
        addinstall { source = "doc/readme",                     destination = "modloader/.data"         }
        addinstall { source = "doc/plugins",                    destination = "modloader/.data/plugins" }
        addinstall { source = "doc/config/config.ini.0",        destination = "modloader/.data"         }
        addinstall { source = "doc/config/modloader.ini.0",     destination = "modloader/.data"         }
        addinstall { source = "doc/config/plugins.ini.0",       destination = "modloader/.data"         }
        addinstall { source = "doc/CHANGELOG.md",               destination = "modloader/.data"         }
        addinstall { source = "doc/Command Line Arguments.md",  destination = "modloader/.data"         }

        -- Dummy cpp file for Premake's generated gmake project,
        -- not adding a file will cause compilation/linking error since it'll try to do something
        configuration "gmake"
            language "C++"
            kind "StaticLib"
            files { "deps/dummy.cpp" }

    project "modloader"
        language "C++"
        kind "SharedLib"
        targetname "modloader"
        targetextension ".asi"
        binarydir ""
        addinstall( { isdir = false, source = "bin/modloader.asi", destination = "./" } )
        links { "addr", "shlwapi", "dbghelp" }
        setupfiles "src/core"
        
        
    -- ordered by time taken to compile
    addplugin "std.movies"
    addplugin "std.scm"
    addplugin "std.sprites"
    addplugin "std.fx"
    addplugin "std.text"
    addplugin "std.bank"
    addplugin "std.stream"
    addplugin "std.asi"

