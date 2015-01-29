/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

//
struct gtadat_traits : public data_traits
{
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = true;     // Is the sections of this data file different on each line?
    

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "level file"; }
        static const char* datafile()   { return "gta.dat"; }       // for readmes
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B905E, dtraits>;
                          
    //
    using dtype      = data_slice<either<uint32_t, std::string>>;                // may be a index to the path (after premerge) or the path itself (before premerge)
    using key_type   = std::pair<uint32_t, uint32_t>;                            // .first is section, .second is either a global index (after premerge) or
                                                                                 //   a local index (as in this->index) (before premere)
    using value_type = gta3::data_section<dtype, dtype, dtype, dtype, dtype, dtype, dtype, dtype, dtype>;

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("IMG", "TEXDICTION", "MODELFILE",    // ORDER HERE MATTERS VERY MUCH!!
                                                       "IDE", "COLFILE", "IPL", "HIERFILE", // Notice COLFILE comes after IDE, that's right!
                                                       "SPLASH", "EXIT");                   // !COLFILE needs the definitions available!
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }

    key_type key_from_value(const value_type& a)
    {
        // During premerge we have a global index in the variant, otherwise we should increment the local index
        if(auto* index = get<uint32_t>(&a.get_slice<dtype>().get<0>()))
            return key_type(a.section()->id, *index);
        else
            return key_type(a.section()->id, ++this->index);
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
            if(section == colsec)   // for COLFILE section, skip the '0' integer after the section specifier
                it = std::find_if(std::find_if(it, end, isspace), end, std::not1(isspace));

            if(it != end)   // the iterator should point to the path at this moment
            {
                setup_data_value(data, section, NormalizePath(std::string(it, end)));
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
        if(data.section() == colsec)            // COLFILE has a '0' integer after it.
            line.append("0").push_back(' ');    // It actually means the colpool index 0, aka generic collisions.
        line.append(path_from_index(get<uint32_t>(data.get_slice<dtype>().get<0>())));

        return true;
    }

    // Set ups the value_type from the specified section and specified argument (string or uint32_t)
    template<class Arg>
    static value_type& setup_data_value(value_type& data, const gta3::section_info* section, Arg&& arg)
    {
        // Since we are setting the data_section and the data_slice manually, we need to call force_section instead of as_section.
        // Then we get the working slice for the section and manually set it's content
        data.force_section(section);
        auto& slice = data.get_slice<dtype>();
        slice.reset();                          // must clear content before setting the slice
        slice.set<0>(std::forward<Arg>(arg));
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
            key_type key = key_from_value(setup_data_value(data, it->second.section(), index_for_path(get<std::string>(it->second.get_slice<dtype>().get<0>()))));
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
            // Turns the index as high as possible while inserting lines from a readme
            // That's because the readme store kinda of merges with a store similar to the default one and
            // we'd conflict keys with the same local index of 0
            this->index = (std::numeric_limits<decltype(index)>::max)() / 2;
        }

    protected:
   
        uint32_t index = 0; // local indexing before premerge

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

// Level File Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    // Mergers for gta.dat and default.dat
    plugin_ptr->AddMerger<gtadat_store>("gta.dat", true, true, false, no_reinstall);
    plugin_ptr->AddMerger<gtadat_store>("default.dat", true, true, false, no_reinstall);

    // Readme reader for gta.dat
    plugin_ptr->AddReader<gtadat_store>([](const std::string& line) -> maybe_readable<gtadat_store>
    {
        // To match a gta.dat line we need the section specifier followed by a single space
        // then a DIRECTORY/ followed by anything until the extension, which should be a valid one.
        static auto regex = make_regex(R"___(^(?:IDE|IPL|IMG|COLFILE 0|TEXDICTION|MODELFILE|HIERFILE) \w+[\\/].*\.(?:IDE|IPL|ZON|IMG|COL|TXD|DFF)\s*$)___", 
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
