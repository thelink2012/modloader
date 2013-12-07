/*
 *  Modloader Plugin Base File
 *  Use this (copy-pasting) to help you in starting a plugin project
 * 
 */
#include <modloader.hpp>
#include <modloader_util.hpp>
using modloader::GetArrayLength;

/*
 *  The plugin object
 */
class CThePlugin : public modloader::CPlugin
{
    public:
        static const int default_priority = 50;
        
        const char* GetName();
        const char* GetAuthor();
        const char* GetVersion();
        int OnStartup();
        int OnShutdown();
        int CheckFile(const modloader::ModLoaderFile& file);
        int ProcessFile(const modloader::ModLoaderFile& file);
        int PosProcess();
        
        const char** GetExtensionTable();

} plugin;

/*
 *  Export plugin object data
 */
extern "C" __declspec(dllexport)
void GetPluginData(modloader_plugin_t* data)
{
    modloader::RegisterPluginData(plugin, data);
    plugin.data->priority = plugin.default_priority;
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
int CThePlugin::OnStartup()
{
    /* return 0 for sucess */
}

int CThePlugin::OnShutdown()
{
    /* return 0 for success */
}

/*
 *  Check if the file is the one we're looking for
 */
int CThePlugin::CheckFile(const modloader::ModLoaderFile& file)
{
    /* return MODLOADER_NO or MODLOADER_YES */
}

/*
 * Process the replacement
 */
int CThePlugin::ProcessFile(const modloader::ModLoaderFile& file)
{
    /* return 0 for success and 1 for failure */
}

/*
 * Called after all files have been processed
 */
int CThePlugin::PosProcess()
{
    /* return 0 for success and 1 for failure */
}
