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
    trigger     = "debug-symbols",
    description = "Makes a release which contains public debug symbols, which allows better user debugging",
}

newoption {
    trigger     = "toolset",
    value       = "tools",
    description = "The toolset used to compile the release build",
    allowed     = {
        { "vs2013", "Microsoft Visual Studio 2013"  },
        { "gcc",    "GNU Compiler Collection"       }
    }
}


toolset     = _OPTIONS["toolset"]
action      = (toolset == "gcc" and "gmake" or toolset)
compiler    = (toolset == "gcc" and "gcc" or "cl")
build       = (toolset == "gcc" and "make" or "msbuild")

function main()

    local debugsymbols  = _OPTIONS["debug-symbols"]

    if not toolset then
        print("No toolset defined.\nAborting.")
        exit()
    end

    local install = function()
        print "Making release directory tree..."
        execute("premake5 install ./release/")
        os.copyfile("./release/modloader/.data/Readme.md", "./release/Readme.txt")
        os.copyfile("./release/modloader/.data/Leia-me.md", "./release/Leia-me.txt")
    end

    print "Cleaning workspace..."
    execute("premake5 clean")

    print "Generating build files..."
    if toolset == "gmake" then
        execute(string.format("premake5 %s --cc=%s --outdir=build_temp", action, compiler))
    else
        execute(string.format("premake5 %s --outdir=build_temp", action))
    end

    print "Building..."
    if build == "msbuild" then
    
        execute("msbuild build_temp/modloader.sln /p:configuration=Release /p:platform=Win32")

        -- Install THEN move pdbs
        install()
        if debugsymbols then
            pdbmove()
        end
        
    elseif compiler == "gcc" then

        local cwd = os.getcwd()
        os.chdir("build_temp")
        execute("mingw32-make CC=gcc")
        os.chdir(cwd)

        -- Strip binaries THEN install
        if not debugsymbols then
            gccstrip()
        end
        install()
        
    else
        print("Internal error")
        exit()
    end

    os.rmdir("build_temp")
end




function pdbmove()
    print("Moving PDB files into release...")
    pdbcopy("bin", "release", false)
    pdbcopy("bin/plugins", "release/modloader/.data/plugins", true)
end

function pdbcopy(src, dest, recursive)
    local cwd = os.getcwd()
    os.chdir(src)
    for i, file in ipairs(os.matchfiles(recursive and "**.pdb" or "*.pdb")) do
        execute(string.format([[pdbcopy "%s" "%s" -p]], file, cwd .. '/' .. dest .. '/' .. file))
    end
    os.chdir(cwd)  
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





function execute(command)
    os.execute(command)
end

function exit()
    os.exit()
end


