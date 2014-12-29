/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using namespace std::placeholders;

// TODO refresh

//
struct carmods_traits : gta3::data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "vehicle upgrades"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B65BE, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // Wheel Section Slice
    using wheel_type = data_slice<int, delimopt,
                                       std::string, std::string, std::string, std::string, std::string,
                                       std::string, std::string, std::string, std::string, std::string, 
                                       std::string, std::string, std::string, std::string, std::string>;

    // Mods Section Slice
    using mods_type = data_slice<std::string, delimopt,
                                       std::string, std::string, std::string, std::string, std::string, std::string,
                                       std::string, std::string, std::string, std::string, std::string, std::string,
                                       std::string, std::string, std::string, std::string, std::string, std::string>;

    // Link Section Slice
    using link_type = data_slice<std::string, std::string>;
                                        
    //
    using value_type = gta3::data_section<wheel_type, mods_type, link_type>;

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info("wheel", "mods", "link");
        return sections.data();
    }

    //
    struct key_type
    {
        using link_type   = std::pair<std::size_t, std::size_t>;    // Hash of two models
        using mods_type   = std::size_t;                            // Hash of a model
        using wheel_type = int;                                     // Index of a model
        either<link_type, mods_type, wheel_type> e;

        struct check_leq : either_static_visitor<bool>
        {
            template<class T, class U>
            bool operator()(const T& a, const U& b) const
            { throw std::invalid_argument("check_leq<T,U>"); }

            // Link needs a special less than operation...
            // If the links are swapped, then it's the same key, otherwise assume the first model as the key
            bool operator()(const link_type& a, const link_type& b) const
            { return (a.first == b.second? false : a.first < b.first); }

            template<class T>
            bool operator()(const T& a, const T& b) const
            { return a < b; }
        };

        key_type() = default;
        key_type(int id) : e(id) {}
        key_type(std::size_t hash) : e(hash) {}
        key_type(std::size_t l1, std::size_t l2) : e(link_type(l1, l2)) {}
        key_type(const key_type&) = default;

        bool operator<(const key_type& rhs) const
        {
            if(this->e.which() == rhs.e.which())
                return apply_visitor(check_leq(), this->e, rhs.e);
            return (this->e.which() < rhs.e.which());
        }

        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(this->e);
        }
    };

    // key_from_value
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        key_type operator()(const wheel_type& slice) const
        { return key_type(get<0>(slice)); }

        key_type operator()(const mods_type& slice) const
        { return key_type(modloader::hash(get<0>(slice))); }

        key_type operator()(const link_type& slice) const
        { return key_type(modloader::hash(get<0>(slice)), modloader::hash(get<1>(slice))); }

        key_type operator()(const either_blank& slice) const
        { throw std::invalid_argument("blank type"); }
    };

    key_type key_from_value(const value_type& value)
    {
        return value.apply_visitor(key_from_value_visitor());
    }
};

//
using carmods_store = gta3::data_store<carmods_traits, std::map<
                        carmods_traits::key_type, carmods_traits::value_type
                        >>;


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const carmods_traits::value_type&)
        {
            return carmods_traits::sections();
        }
    }
}

// Vehicle Upgrades Merger
static auto xinit = initializer(std::bind(&DataPlugin::AddMerger<carmods_store>, _1, "carmods.dat", true, no_reinstall));

