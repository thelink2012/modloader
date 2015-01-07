/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using namespace std::placeholders;
using std::tuple;
using std::vector;

//
struct carcols_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "vehicle colours"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B68AB, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // Section slices
    using col_type  = data_slice<rgb, optional<udata<int>>>;
    using car_type  = data_slice<modelname, vector<tuple<uint16_t, uint16_t>>>;
    using car4_type = data_slice<modelname, vector<tuple<uint16_t, uint16_t, uint16_t, uint16_t>>>;
                                        
    //
    using key_type   = std::pair<bool, size_t>;  // .first is boolean indicating either col_type or car/car4_type and .second is hash/lineid
    using value_type = gta3::data_section<col_type, car_type, car4_type>;

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("col", "car", "car4");
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }

    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<size_t>
    {
        template<class T>
        size_t operator()(const T& slice) const
        { return hash_model(get<0>(slice)); }

        size_t operator()(const col_type& slice) const
        { return get(get<1>(slice).get()); }

        size_t operator()(const either_blank& slice) const
        { throw std::invalid_argument("blank type"); }
    };

    key_type key_from_value(const value_type& value)
    {
        static auto colsec = gta3::section_info::by_name(sections(), "col");
        return key_type(value.section() != colsec, value.apply_visitor(key_from_value_visitor()));
    }

    // make setbyline output a error on failure
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        static auto colsec = gta3::section_info::by_name(sections(), "col");

        std::string line2;
        const std::string* linep = &line;

        // Okay so the 'col' section has a bug in it, Rockstar used a damn '.' instead of a ','
        if(section == colsec)
        {
            linep = &line2;
            line2 = line;
            std::replace(line2.begin(), line2.end(), '.', ' ');
        }

        if(data_traits::setbyline(store, data, section, *linep))
        {
            if(data.section() == colsec)    // assign line index to color info
                data.get_slice<col_type>().set<1>(make_udata<int>(store.traits().colindex++));
            return true;
        }

        return false;
    }



    public: // traits data

        int colindex = 0;   // Line index we are going tho for 'col' section

        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(this->colindex);
        }
};

//
using carcols_store = gta3::data_store<carcols_traits, std::map<
                        carcols_traits::key_type, carcols_traits::value_type
                        >>;


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const carcols_traits::value_type&)
        {
            return carcols_traits::sections();
        }
    }
}

// Vehicle Upgrades Merger
static auto xinit = initializer(std::bind(&DataPlugin::AddMerger<carcols_store, std::function<void()>>, _1,
    "carcols.dat", true, false, false, reinstall_since_load, injector::cstd<void()>::call<0x5B6890>));
