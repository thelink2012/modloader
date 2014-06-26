/*
* Copyright (C) 2013-2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under GNU GPL v3, see LICENSE at top level directory.
*
*/
#include <map>
#include <modloader/modloader.hpp>
#include <modloader/gta3/gta3.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/path.hpp>
#include <modloader/util/injector.hpp>
using namespace modloader;
using namespace injector;

/*
 *  grass_dff_detour  
 *      Grass cannot get detoured using the standard mod loader detourer because it is one call for all grass files
 *      Let's just create this specialized detour
 */
template<uintptr_t addr>
struct grass_dff_detour : RwStreamOpenDetour<addr>
{
    protected:
        using base = RwStreamOpenDetour<addr>;

        typedef RwStreamOpenDetour<addr> MyBase;
        struct grass_store {
            std::string grass[2][4];
        };

        // Gets the filepath for the grass i,j (i is the first number on the grass filename and j the second)
        static std::string& grass(int i, int j) { static grass_store x; return x.grass[i][j-1]; }

        // This hook just wraps around the base hook, we'll call it
        static void* hook(typename base::func_type fun, int& a, int& b, const char*& fpath)
        {
            int i, j;

            // Get d grazz
            if(sscanf(&fpath[GetLastPathComponent(std::string(fpath))], "grass%d_%d.dff", &i, &j) >= 2)
                MyBase::store().path = grass(i,j);
            else
                MyBase::store().path.clear();

            // Forward to base
            return MyBase::hook(fun, a, b, fpath);
        }

    public:
        // Constructors, move constructors, assigment operators........
        grass_dff_detour() = default;
        grass_dff_detour(const grass_dff_detour&) = delete;
        grass_dff_detour(grass_dff_detour&& rhs) : MyBase(std::move(rhs)) {}
        grass_dff_detour& operator=(const grass_dff_detour& rhs) = delete;
        grass_dff_detour& operator=(grass_dff_detour&& rhs)
        {
            MyBase::operator=(std::move(rhs)); return *this;
        }

        // Makes the hook
        void make_call()
        {
            base::function_hooker::make_call(hook);
        }

        void setfile(std::string path, int i, int j)
        {
            make_call();
            grass(i, j) = std::move(path);
        }
};


/*
 *  The plugin object
 */
class FxPlugin : public modloader::basic_plugin
{
    private:
        std::map<size_t, modloader::file_overrider> ovmap;      // Map of files to be overriden, map<hash, overrider>
        grass_dff_detour<0x5DD272> grass_detour;                // Grass needs a specialized detouring

    protected:
        // Adds a detour for the file with the specified hash to the overrider subsystem
        template<class ...Args> file_overrider& AddDetour(size_t hash, Args&&... args)
        {
            return ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(hash),
                std::forward_as_tuple(tag_detour, std::forward<Args>(args)...)
                ).first->second;
        }

        // Add dummy detour (does nothing ever) to the overrider subsystem
        // Useful for files that should've been handled by us but won't take any effect in game because they're unused files
        file_overrider& AddDummy(size_t hash)
        {
            return ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(hash),
                std::forward_as_tuple()
                ).first->second;
        }

        // Add a manual detourer to the overrider subsystem
        file_overrider& AddManualDetour(size_t hash, const file_overrider::params& p)
        {
            return ovmap.emplace(std::piecewise_construct,
                std::forward_as_tuple(hash), std::forward_as_tuple(p)
                ).first->second;
        }

        // Add grass overrider to the overrider subsystem, grass needs special treatment
        void AddGrass(int i, int j)
        {
            char buf[64]; sprintf(buf, "grass%d_%d.dff", i, j);
            AddManualDetour(modloader::hash(buf), file_overrider::params(nullptr)).OnChange([this, i, j](const modloader::file* f)
            {
                grass_detour.setfile(f? f->FileBuffer() : "", i, j);
            });
        }

    public:
        // Finds overrider for the file with the specified hash, rets null if not found
        file_overrider* FindOverrider(size_t hash)
        {
            auto it = ovmap.find(hash);
            if(it != ovmap.end()) return &it->second;
            return nullptr;
        }

    public:
        // Standard plugin methods
        bool OnStartup();
        bool OnShutdown();
        const info& GetInfo();
        int  GetBehaviour(modloader::file&);
        bool InstallFile(const modloader::file&);
        bool ReinstallFile(const modloader::file&);
        bool UninstallFile(const modloader::file&);
        

} fx_plugin;

REGISTER_ML_PLUGIN(::fx_plugin)


/*
 *  FxPlugin::GetInfo
 *      Returns information about this plugin 
 */
const FxPlugin::info& FxPlugin::GetInfo()
{
    static const char* extable[] = { "dff", "txd", "fxp", 0 };
    static const info xinfo      = { "FX Loader", "Reloaded 0.1", "LINK/2012", -1, extable };
    return xinfo;
}


/*
 *  FxPlugin::OnStartup
 *      Startups the plugin
 */
bool FxPlugin::OnStartup()
{
    if(gvm.IsSA())
    {
        // File overrider params
        const auto reinstall_since_start = file_overrider::params(true, true, true, true);
        const auto reinstall_since_load  = file_overrider::params(true, true, false, true);
        const auto no_reinstall          = file_overrider::params(nullptr);

        // Files hashes
        const auto hud_txd           = modloader::hash("hud.txd");
        const auto particle_txd      = modloader::hash("particle.txd");
        const auto effects_fxp       = modloader::hash("effects.fxp");
        const auto effectspc_txd     = modloader::hash("effectspc.txd");
        const auto fonts_txd         = modloader::hash("fonts.txd");
        const auto pcbtns_txd        = modloader::hash("pcbtns.txd");
        const auto fronten1_txd      = modloader::hash("fronten1.txd");
        const auto fronten2_txd      = modloader::hash("fronten2.txd");
        const auto fronten3_txd      = modloader::hash("fronten3.txd");
        const auto fronten_pc_txd    = modloader::hash("fronten_pc.txd");
        const auto vehicle_txd       = modloader::hash("vehicle.txd");
        const auto plant1_txd        = modloader::hash("plant1.txd");
        const auto grass0_1_dff      = modloader::hash("grass0_1.dff");
        const auto grass0_2_dff      = modloader::hash("grass0_2.dff");
        const auto grass0_3_dff      = modloader::hash("grass0_3.dff");
        const auto grass0_4_dff      = modloader::hash("grass0_4.dff");
        const auto grass1_1_dff      = modloader::hash("grass1_1.dff");
        const auto grass1_2_dff      = modloader::hash("grass1_2.dff");
        const auto grass1_3_dff      = modloader::hash("grass1_3.dff");
        const auto grass1_4_dff      = modloader::hash("grass1_4.dff");

        // Unused (or mostly unused) files dff/txd files in the the models/ folder
        const size_t unused_table[] =
        {
            modloader::hash("misc.txd"),     modloader::hash("wheels.txd"),
            modloader::hash("wheels.dff"),   modloader::hash("zonecylb.dff"),
            modloader::hash("hoop.dff"),     modloader::hash("arrow.dff"),
            modloader::hash("air_vlo.dff"),  modloader::hash("plant1.dff"),
            modloader::hash("grass2_1.dff"), modloader::hash("grass2_2.dff"),
            modloader::hash("grass2_3.dff"), modloader::hash("grass2_4.dff"),
            modloader::hash("grass3_1.dff"), modloader::hash("grass3_2.dff"),
            modloader::hash("grass3_3.dff"), modloader::hash("grass3_4.dff"),
            modloader::hash("peds.col"),     modloader::hash("vehicles.col"), 
            modloader::hash("weapons.col"),
        };

        auto ReloadFonts = []
        {
            injector::cstd<void()>::call(0x7189B0); // CFont::Shutdown 
            injector::cstd<void()>::call(0x5BA690); // CFont::Initialise
        };

        auto ReloadFronten = []
        {
            void* menumgr = lazy_pointer<0xBA6748>().get();           // FrontEndManager
            injector::thiscall<void(void*)>::call(0x574630, menumgr); // CMenuManager::UnloadTextures
            injector::thiscall<void(void*)>::call(0x572EC0, menumgr); // CMenuManager::LoadTextures
        };

        auto ReloadHud = []
        {
            injector::cstd<void()>::call(0x588850); // CHud::Shutdown 
            injector::cstd<void()>::call(0x5827D0); // CRadar::LoadTextures
            injector::cstd<void()>::call(0x5BA850); // CHud::Initialise
            injector::cstd<void()>::call(0x5827D0); // CRadar::LoadTextures
        };


        // Insert unused dff/txd files in the models/ folder there, so the img plugin won't load those
        for(auto& unused : unused_table) AddDummy(unused);

        // Detouring for HUD
        AddDetour(hud_txd, reinstall_since_load, LoadTxdDetour<0x5BA865>(), ReloadHud);

        // Detouring for fonts
        AddDetour(fonts_txd, reinstall_since_start, LoadTxdDetour<0x5BA6A4>(), ReloadFonts);
        AddDetour(pcbtns_txd, reinstall_since_start, LoadTxdDetour<0x5BA7D4>(), ReloadFonts);

        // Detouring for frontend textures
        AddDetour(fronten1_txd, reinstall_since_start, LoadTxdDetour<0x572F1E>(), ReloadFronten);
        AddDetour(fronten2_txd, reinstall_since_start, LoadTxdDetour<0x573040>(), ReloadFronten);
        AddDetour(fronten_pc_txd, reinstall_since_start, LoadTxdDetour<0x572FB5>(), ReloadFronten);
        AddDummy (fronten3_txd);

        // Detouring for common vehicle textures
        // No reloading because too many references to the textures in clump materials, maybe in the future
        AddDetour(vehicle_txd, no_reinstall, LoadTxdDetour<0x5B8F5E>());

        // Detouring for particle textures
        // No reloading because too many references to the particle textures in allocated objects
        AddDetour(particle_txd, no_reinstall, LoadTxdDetour<0x5BF8B7>());

        // Detouring for particle effects
        // No reloading because reloading those barely works, it causes the game to be unstable, crashing some times
        AddDetour(effectspc_txd, no_reinstall, LoadTxdDetour<0x5C248F>());
        AddDetour(effects_fxp, no_reinstall, OpenFileDetour<0x5C24B1>());
        
        // Detouring for grass
        // No reloading because leaked texture (@0xC09174) after shutdown, maybe in the future
        AddDetour(plant1_txd, no_reinstall, LoadTxdDetour<0x5DD95F>());
        AddGrass(0, 1); AddGrass(0, 2); AddGrass(0, 3); AddGrass(0, 4);
        AddGrass(1, 1); AddGrass(1, 2); AddGrass(1, 3); AddGrass(1, 4);

        return true;
    }
    return false;
}

/*
 *  FxPlugin::OnShutdown
 *      Shutdowns the plugin
 */
bool FxPlugin::OnShutdown()
{
    return true;
}

/*
 *  FxPlugin::GetBehaviour
 *      Gets the relationship between this plugin and the file
 */
int FxPlugin::GetBehaviour(modloader::file& file)
{
    if(this->FindOverrider(file.hash))
    {
        file.behaviour = file.hash;
        return MODLOADER_BEHAVIOUR_YES;
    }
    return MODLOADER_BEHAVIOUR_NO;
}

/*
 *  FxPlugin::InstallFile
 *      Installs a file using this plugin
 */
bool FxPlugin::InstallFile(const modloader::file& file)
{
    if(auto ov = this->FindOverrider(file.hash)) return ov->InstallFile(file);
    return false;
}

/*
 *  FxPlugin::ReinstallFile
 *      Reinstall a file previosly installed that has been updated
 */
bool FxPlugin::ReinstallFile(const modloader::file& file)
{
    if(auto ov = this->FindOverrider(file.hash)) return ov->ReinstallFile();
    return false;
}

/*
 *  FxPlugin::UninstallFile
 *      Uninstall a previosly installed file
 */
bool FxPlugin::UninstallFile(const modloader::file& file)
{
    if(auto ov = this->FindOverrider(file.hash)) return ov->UninstallFile();
    return false;
}
