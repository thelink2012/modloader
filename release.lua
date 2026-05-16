--[[
    release.lua
        Prepares a public release
        Use 'premake5 --file=release.lua --help' for help
--]]

newaction {
    trigger     = "prepare",
    description = "Automatically makes 'n builds a release build, preparing it to be zipped into a public release",
    execute     = function() main() end
}

newoption {
    trigger     = "toolset",
    value       = "tools",
    description = "The toolset used to compile the release build (vs20xx or gcc)",
}

newoption {
    trigger     = "final-release",
    description = "Public release build."
}

toolset     = _OPTIONS["toolset"]
final_flag  = _OPTIONS["final-release"] and " --final-release" or ""
action      = (toolset == "gcc" and "gmake" or toolset)
compiler    = (toolset == "gcc" and "gcc" or "cl")
build       = (toolset == "gcc" and "make" or "msbuild")

function main()

    if not toolset then
        print("No toolset defined.\nAborting.")
        exit()
    end

    if toolset ~= "gcc" and not toolset:match("^vs") then
        print("Unsupported toolset '" .. toolset .. "' (use a vs* premake action or gcc).\nAborting.")
        exit()
    end

    require_tools()

    local install = function()
        print "Making release directory tree..."
        execute("premake5 install ./release/binaries/")
        os.copyfile("./release/binaries/modloader/.data/Readme.md", "./release/binaries/Readme.txt")
        os.copyfile("./release/binaries/modloader/.data/Leia-me.md", "./release/binaries/Leia-me.txt")
        os.mkdir("./release/binaries/modloader/.profiles")
    end

    print "Cleaning workspace..."
    execute("premake5 clean")

    print "Generating build files..."
    if toolset == "gcc" then
        execute(string.format("premake5 %s --cc=%s --outdir=build_temp%s", action, compiler, final_flag))
    else
        execute(string.format("premake5 %s --outdir=build_temp%s", action, final_flag))
    end

    print "Building..."
    if build == "msbuild" then

         -- also use 'set CL=/MP' at release.bat
        execute("msbuild build_temp/modloader.sln /p:configuration=Release /p:platform=Win32 /m")

        install()
        pdbpackage()

    elseif compiler == "gcc" then

        local cwd = os.getcwd()
        os.chdir("build_temp")
        execute("mingw32-make CC=gcc")
        os.chdir(cwd)

        -- Strip binaries (ALWAYS)
        gccstrip()
        -- striping is not related to symbols, to have symbols send --export-all-symbols to the linker

        install()

    else
        print("Internal error")
        exit()
    end

    os.rmdir("build_temp")
end

function pdbpackage()
    print("Packaging stripped debug symbols (release/symbols/)...")

    os.mkdir("release/symbols")
    os.mkdir("release/symbols/modloader/.data/plugins/gta3")

    execute('pdbcopy "bin/modloader.pdb" "release/symbols/modloader.pdb" -p')

    for i, file in ipairs(os.matchfiles("bin/plugins/gta3/*.pdb")) do
        local name = path.getname(file)
        execute(string.format(
            'pdbcopy "bin/plugins/gta3/%s" "release/symbols/modloader/.data/plugins/gta3/%s" -p',
            name, name
        ))
    end
end

function gccstrip()
    print "Stripping GCC symbols..."

    local cwd = os.getcwd()
    os.chdir("bin")
    for i, file in ipairs(os.matchfiles("*.asi")) do
        execute(string.format([[strip "%s"]], file))
    end
    for i, file in ipairs(os.matchfiles("**.dll")) do
        execute(string.format([[strip "%s"]], file))
    end
    os.chdir(cwd)
end

function require_tools()
    require_on_path("premake5",
        "Install Premake 5 and add it to PATH.")

    if build == "msbuild" then
        require_on_path("msbuild",
            "Install Visual Studio with MSBuild, then run this from the x86 Native Tools Command Prompt.")
        require_on_path("pdbcopy",
            "Install the Debugging Tools for Windows component from the Windows SDK," ..
            " then add %ProgramFiles(x86)%\\Windows Kits\\10\\Debuggers\\x86 to PATH.")
    elseif compiler == "gcc" then
        require_on_path("mingw32-make",
            "Install MinGW and add mingw32-make to PATH.")
    end
end

function require_on_path(name, hint)
    if not os.ishost("windows") then
        return
    end
    local found = os.outputof("where " .. name .. " 2>nul")
    if not found or found == "" then
        print("Error: " .. name .. " is not on PATH.")
        if hint then
            print(hint)
        end
        exit()
    end
end

function execute(command)
    local result = os.execute(command)
    if result == 0 or result == true then
        return
    end
    print("Command failed: " .. command .. "\nAborting.")
    exit()
end

function exit()
    os.exit()
end
