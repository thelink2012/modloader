/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

namespace {
/*
 *  readme_key 
 *      This key is special because two keys will never be the same.
 *      This is important for data taken from readme files and later merged into the default store.
 */
struct readme_key
{
    readme_key() : mykey(getkey()) {}
    readme_key(const readme_key&) : mykey(getkey()) {}
    readme_key(readme_key&& rhs) { this->mykey = rhs.mykey; rhs.mykey = -1; }

    // Note: This method exists because boost asks for it, don't invoke it directly.
    readme_key& operator=(const readme_key&) = default;

    bool operator<(const readme_key& rhs) const  { return this->mykey < rhs.mykey; }
    bool operator==(const readme_key& rhs) const { return this->mykey == rhs.mykey; }

private:
    uint32_t mykey;

    static int getkey()
    {
        static uint32_t i = 0;
        return ++i;
    }

public:
    template<class Archive>
    void serialize(Archive&)
    { /* dont save anything, key is per instance */ }
};
}

//
struct gtadat_traits : public data_traits
{
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = true;     // Is the sections of this data file different on each line?
    

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "level file"; }
        static const char* datafile() // for readmes
        {
            if(gvm.IsSA()) return "gta.dat";
            if(gvm.IsVC()) return "gta_vc.dat";
            if(gvm.IsIII()) return "gta3.dat";
            return nullptr;
        }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B905E, dtraits>;
                          
    //
    using dtype      = data_slice<either<uint32_t, std::string>, uint32_t>;      //< <0> = may be a index to the path (after premerge) or the path itself (before premerge)
                                                                                 //^ <1> = for COLFILE, the level index, otherwise 0.
    using key_type   = std::pair<uint32_t, either<uint32_t, readme_key>>;        // .first is section, .second is either a global index (after premerge) or
                                                                                 //   a local index (as in this->index) (before premere)
    using value_type = gta3::data_section<dtype, dtype, dtype, dtype, dtype, dtype, dtype, dtype, dtype, dtype, dtype>;

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("IMG", "CDIMAGE", "TEXDICTION", "MODELFILE",     // ORDER HERE MATTERS VERY MUCH!!
                                                       "IDE", "COLFILE",  "MAPZONE", "IPL", "HIERFILE", // Notice COLFILE comes after IDE, that's right!
                                                       "SPLASH", "EXIT");                               // !COLFILE needs the definitions available!
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }

    key_type key_from_value(const value_type& a)
    {
        // During premerge we have a global index in the variant, otherwise we should increment the local index
        if(auto* index = get<uint32_t>(&a.get_slice<dtype>().get<0>()))
            return key_type(a.section()->id, *index);
        else
        {
            if(this->is_readme)
                return key_type(a.section()->id, readme_key());
            else
                return key_type(a.section()->id, ++this->index);
        }
    }

    static const gta3::section_info* section_by_line(const gta3::section_info* sections, const std::string& line)
    {
        return gta3::section_info::by_name(sections, line, -1);
    }

    /*
     *  Manually parses a gta.dat line and puts into the 'data' slice
     */
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        static auto colsec = gta3::section_info::by_name(sections(), "COLFILE");

        auto isspace = std::function<int(int)>(::isspace);  // functor wrapper for std::isspace
        auto it      = line.begin();
        auto end     = line.end();

        it = std::find_if(it + section->len, end, std::not1(isspace));  // skip section specifier (IMG, IDE, COLFILE, ...)
        if(it != end)
        {
            uint32_t level = 0;

            if(section == colsec)   // for COLFILE section, get the level id after the section specifier
            {
                size_t beg_index = std::distance(line.begin(), it);
                it = std::find_if(std::find_if(it, end, isspace), end, std::not1(isspace));
                size_t end_index = std::distance(line.begin(), it);
                try
                {
                    level = std::stoi(line.substr(beg_index, end_index - beg_index));
                }
                catch(const std::logic_error&)
                {
                    // do nothing about it, let it go
                }
            }

            if(it != end)   // the iterator should point to the path at this moment
            {
                setup_data_value(data, section, NormalizePath(std::string(it, end)), level);
                return true;
            }
        }

        return false;
    }

    /*
     *  Manually puts a gta.dat line
     *  Data must have been processed tho premerge, turning the slice into a index slice not a string slice
     */
    template<class StoreType>
    static bool getline(const key_type& key, const value_type& data, std::string& line)
    {
        static auto colsec = gta3::section_info::by_name(sections(), "COLFILE");

        line.assign(data.section()->name).push_back(' ');
        if(data.section() == colsec)
        {
            uint32_t level = data.get_slice<dtype>().get<1>();
            line.append(std::to_string(level)).push_back(' ');
        }
        line.append(path_from_index(get<uint32_t>(data.get_slice<dtype>().get<0>())));

        return true;
    }

    // Set ups the value_type from the specified section and specified argument (string or uint32_t)
    template<class Arg>
    static value_type& setup_data_value(value_type& data, const gta3::section_info* section, Arg&& arg, uint32_t level)
    {
        // Since we are setting the data_section and the data_slice manually, we need to call force_section instead of as_section.
        // Then we get the working slice for the section and manually set it's content
        data.force_section(section);
        auto& slice = data.get_slice<dtype>();
        slice.reset();                          // must clear content before setting the slice
        slice.set<0>(std::forward<Arg>(arg));
        slice.set<1>(level);
        return data;
    }

    // Transform the current container (that uses local indices and a string path as value) into a container that uses global indices
    // and a indice to the path as value
    template<class StoreType>
    bool premerge(StoreType& store)
    {
        using container_type = typename StoreType::container_type;
        using pair_type      = typename StoreType::pair_type;

        container_type newcontainer;

        for(auto it = store.container().begin(); it != store.container().end(); ++it)
        {
            value_type data;
            uint32_t index = index_for_path(get<std::string>(it->second.get_slice<dtype>().get<0>()));
            uint32_t level = it->second.get_slice<dtype>().get<1>();
            key_type key = key_from_value(setup_data_value(data, it->second.section(), index, level));
            newcontainer.emplace(std::move(key), std::move(data));
        }

        store.container() = std::move(newcontainer);
        return true;
    }

    // After the merging process free up the mapping of paths to global indices
    template<class StoreType>
    bool posmerge(StoreType& store)
    {
        mapindex().clear();
        return true;
    }

    template<class StoreType>
    static DataPlugin::readme_data_list<StoreType> query_readme_data(const std::string& filename)
    {
        // Data lines should be only for gta.dat
        if(filename == dtraits::datafile())
            return data_traits::query_readme_data<StoreType>(filename);
        return DataPlugin::readme_data_list<StoreType>();
    }
    

    public:
        void setup_for_readme()
        {
            this->is_readme = true;
        }

    protected:
   
        uint32_t index = 0; // local indexing before premerge
        bool is_readme = false;

        template<class Archive>
        void serialize(Archive& archive)
        { archive(this->index); }

        friend class cereal::access;

    private:

    /*
     *  Something very important in the gta.dat entries are it's order.
     *  The order stuff is loaded is very important, not only sectioned order but the content of each section itself
     *  For example, IPL ordering is super important, mainly because of their placements are saved in the save game by
     *  indice (INST objects which scripts manipulate, ENEXes, and much more).
     *  
     *  So we must make sure the order of the items in a specific section is ordered as it should, this function outputs a indice
     *  that can be used for this ordering.
     *
     *  Notice it saves the path so that if it's received again, it returns the same indice :)
     *
     */

    struct mapindex_t
    {
        uint32_t                                index = 0;
        std::map<std::string, uint32_t>         path2index;
        std::map<uint32_t, const std::string*>  index2path;

        void clear()
        { this->index = 0; this->path2index.clear(); this->index2path.clear(); }
    };

    static mapindex_t& mapindex()
    {
        static mapindex_t map;
        return map;
    }

    // Finds the index for the specified path, if no index associated with it makes a association
    static uint32_t index_for_path(std::string path)
    {
        auto& m = mapindex();
        path = NormalizePath(std::move(path));

        auto it = m.path2index.find(path);
        if(it != m.path2index.end())
            return it->second;
        else
        {
            it = m.path2index.emplace(std::move(path), ++m.index).first;
            return m.index2path.emplace(it->second, &it->first).first->first;
        }
    }

    // Gets the path associated with the specified index, throws a exception if no association is present
    static const std::string& path_from_index(uint32_t index)
    {
        auto p = mapindex().index2path[index];
        if(p == nullptr) throw std::runtime_error("gtadat_traits::path_from_index bug");
        return *p;
    }
};

//
using gtadat_store = gta3::data_store<gtadat_traits, std::map<
                        gtadat_traits::key_type, gtadat_traits::value_type
                        >>;

REGISTER_RTTI_FOR_ANY(gtadat_store);


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const gtadat_traits::value_type&)
        {
            return gtadat_traits::sections();
        }
    }
}

template<uintptr_t addr>
using detour_type = modloader::OpenFileDetour<addr, gtadat_traits::dtraits>;

// Level File Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    const char* maindat = gtadat_traits::dtraits::datafile();

    if(!gvm.IsIII())
    {
        // Mergers for gta.dat and default.dat
        plugin_ptr->AddMerger<gtadat_store>(maindat, true, true, false, no_reinstall);
        plugin_ptr->AddMerger<gtadat_store>("default.dat", true, true, false, no_reinstall);
    }
    else
    {
        auto GetMergedData = plugin_ptr->BindGetMergedData<gtadat_store>(maindat, true, true, false);

        plugin_ptr->AddMerger<gtadat_store>(maindat, tag_detour, no_reinstall,
                    std::forward_as_tuple(
                        detour_type<0x5B905E>(GetMergedData),      // @CFileLoader::LoadLevel
                        detour_type<xIII(0x476534)>(GetMergedData) // @CFileLoader::LoadCollisionFile1
                        ));

        plugin_ptr->AddMerger<gtadat_store>("default.dat", true, true, false, no_reinstall);
    }

    // Readme reader for gta.dat
    plugin_ptr->AddReader<gtadat_store>([](const std::string& line) -> maybe_readable<gtadat_store>
    {
        // To match a gta.dat line we need the section specifier followed by a single space
        // then a DIRECTORY/ followed by anything until the extension, which should be a valid one.
        static auto regex = make_regex(R"___(^(?:IDE|IPL|IMG|COLFILE \d|TEXDICTION|MODELFILE|HIERFILE) \w+[\\/].*\.(?:IDE|IPL|ZON|IMG|COL|TXD|DFF)\s*$)___", 
                                        sregex::ECMAScript|sregex::optimize|sregex::icase); // case insensitive because of the file extension

        if(regex_match(line, regex))
        {
            gtadat_store store;
            store.traits().setup_for_readme();
            if(store.insert(gtadat_store::section_by_line(gtadat_store::sections(), line), line))
                return store;
        }
        return nothing;
    });
});
