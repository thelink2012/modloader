/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::tuple;
using std::vector;

//
struct carcols_traits : public data_traits
{
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "vehicle colours"; }
        static const char* datafile()   { return "carcols.dat"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B68AB, dtraits>;

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
        void serialize(Archive& archive)
        { archive(this->colindex); }
};

//
using carcols_store = gta3::data_store<carcols_traits, std::map<
                        carcols_traits::key_type, carcols_traits::value_type
                        >>;

REGISTER_RTTI_FOR_ANY(carcols_store);

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

// Vehicle Colours Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    if(!gvm.IsSA())
        return;

    auto ReloadColours = injector::cstd<void()>::call<0x5B6890>;
    plugin_ptr->AddMerger<carcols_store>("carcols.dat", true, false, false, reinstall_since_load, gdir_refresh(ReloadColours));

    // Readme reader
    plugin_ptr->AddReader<carcols_store>([](const std::string& line) -> maybe_readable<carcols_store>
    {
        // A pattern for a carcols line is very specific and needs special spacing, it is exactly:
        // <VEHMODEL>REPEAT(  <c1> <c2> [<c3> <c4>])
        static auto regex = make_regex(R"___(^(\w+)\s*(?:((?: (?: \d+){2})+)|((?: (?: \d+){4})+))\s*$)___");

        smatch match;
        if(regex_match(line, match, regex))
        {
            // Oh, the pattern matches with this line!
            if(HasModelInfo())
            {
                if(match.size() == 4 && MatchModelString(match[1])) // matches vehicle model?
                {
                    carcols_store store;
                    if(match[2].length()) store.insert<carcols_traits::car_type>(line);
                    else if(match[3].length()) store.insert<carcols_traits::car4_type>(line);
                    return store;
                }
            }
            else // We're still not able to tell if this is a carcol line, we need the models names! But maybe...
                return maybe<carcols_store>();
        }
        return nothing;
    });
});
