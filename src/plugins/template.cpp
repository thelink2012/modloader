/*
 *  Modloader Plugin Base File
 *  Use this (copy-pasting) to help you in starting a plugin project
 * 
 */
#include <modloader.hpp>

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        bool OnStartup();
        bool OnShutdown();
        bool CheckFile(modloader::ModLoaderFile& file);
        bool ProcessFile(const modloader::ModLoaderFile& file);
        bool PosProcess();
        
        const char** GetExtensionTable();

} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    modloader::RegisterPluginData(plugin, data);
}



/*
 *  Basic plugin informations
 */
const char* CThePlugin::GetName()
{
    /* Return a unique name that identifies the plugin */
}

const char* CThePlugin::GetAuthor()
{
    /* Return your name (e.g. "LINK/2012")*/
}

const char* CThePlugin::GetVersion()
{
    /* Return the plugin version (e.g. "1.0") */
}

const char** CThePlugin::GetExtensionTable()
{
    /* Put the extensions  this plugin handles on @table */
    static const char* table[] = { 0 };
    return table;
}

/*
 *  Startup / Shutdown (do nothing)
 */
bool CThePlugin::OnStartup()
{
    /* return true for sucess */
}

bool CThePlugin::OnShutdown()
{
    /* return true for success */
}

/*
 *  Check if the file is the one we're looking for
 */
bool CThePlugin::CheckFile(modloader::ModLoaderFile& file)
{
    /* return true for yes and false for no */
}

/*
 * Process the replacement
 */
bool CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    /* return true for success and false for failure */
}

/*
 * Called after all files have been processed
 */
bool CThePlugin::PosProcess()
{
    /* return true for success and false for failure */
}
