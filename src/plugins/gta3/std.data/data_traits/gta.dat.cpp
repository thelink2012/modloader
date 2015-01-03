/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using namespace std::placeholders;

//
struct gtadat_traits : gta3::data_traits
{
    static const bool can_cache         = false;    // -> This store cannot be cached because of static indices
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = true;     // Is the sections of this data file different on each line?
    

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "level file"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B905E, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;
                                        
    //
    using dtype      = data_slice<uint32_t>;                // <0> is a index to the path (see path_from_index() function)
    using key_type   = std::pair<uint32_t, uint32_t>;       // .first is section, .second is path index
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
        return key_type(a.section()->id, a.get_slice<dtype>().get<0>());
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
                // Since we are setting the data_section and the data_slice manually, we need to call force_section instead of as_section.
                // Then we get the working slice for the section and manually set it's content
                data.force_section(section);
                auto& slice = data.get_slice<dtype>();
                slice.reset();                          // must clear content before setting the slice
                slice.set<0>(index_for_path(std::string(it, end)));
                return true;
            }
        }

        return false;
    }

    /*
     *  Manually puts a gta.dat line
     */
    template<class StoreType>
    static bool getline(const key_type& key, const value_type& data, std::string& line)
    {
        static auto colsec = gta3::section_info::by_name(sections(), "COLFILE");

        line.assign(data.section()->name).push_back(' ');
        if(data.section() == colsec)            // COLFILE has a '0' integer after it.
            line.append("0").push_back(' ');    // It actually means the colpool index 0, aka generic collisions.
        line.append(path_from_index(data.get_slice<dtype>().get<0>()));

        return true;
    }


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
        uint32_t                                index;
        std::map<std::string, uint32_t>         path2index;
        std::map<uint32_t, const std::string*>  index2path;
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
static auto xinit1 = initializer(std::bind(&DataPlugin::AddMerger<gtadat_store>, _1, "gta.dat", true, true, false, no_reinstall));
static auto xinit2 = initializer(std::bind(&DataPlugin::AddMerger<gtadat_store>, _1, "default.dat", true, true, false, no_reinstall));
