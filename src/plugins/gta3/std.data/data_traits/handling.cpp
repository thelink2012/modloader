/*
 * Copyright (C) 2014-2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;
using namespace std::placeholders;
using std::string;
using std::tuple;
using std::shared_ptr;
using std::static_pointer_cast;
using data_slice_ptr = shared_ptr<data_slice_base>;

// Now, handling.cfg has some broken formating, so let's have a type that can handle it
// Well I'm to lazy to do a proper parser for it, so just let's assume a string when the broken token cannot be readen
// Don't use it everywhere, so the readme parser will break, allowing anything to pass. Use it only where you saw a token mistake.
template<class T>
using fixtok =  either<T, string>;

static bool reading_from_readme = false;

//
struct handling_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = true;     // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "vehicles handling"; }
        static const char* datafile()   { return "handling.cfg"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5BD850, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;


    // Section slices
    //      Notice:
    //          + main_type uses hex<uint64_t> due to R* using a higher than 32 bit value in their handling (their sscanf could handle it properly)
    //          + notice fixtok<real_t> on plane_type, it's due to '$ RCRAIDER' broken float, having a 's' suffix
    using anim_type  = data_slice<char, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, int>;
    using main_type  = data_slice<string, real_t, real_t, real_t, real_t, real_t, real_t, int, real_t, real_t, real_t, int, real_t, real_t, real_t, char, char, real_t, real_t, char, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, int, hex<uint64_t>, hex<uint32_t>, char, char, int>;
    using boat_type  = data_slice<char, string, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t>;
    using bike_type  = data_slice<char, string, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t>;
    using plane_type = data_slice<char, string, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, fixtok<real_t>, real_t, real_t, real_t, real_t, real_t, real_t, real_t>;

    // Aliases and constants related to section slices
    static const size_t main_anim_id = (std::tuple_size<main_type::tuple_type>::value - 1);  // index of either<int, anim_ptr> at main_type
    using main_ptr   = shared_ptr<main_type>;
    using boat_ptr   = shared_ptr<boat_type>;
    using bike_ptr   = shared_ptr<bike_type>;
    using plane_ptr  = shared_ptr<plane_type>;
    using anim_ptr   = shared_ptr<anim_type>;
    using data_tuple = tuple<main_ptr, boat_ptr, bike_ptr, plane_ptr>;
    using final_type = data_slice<data_tuple>;      // final_type is a intermediate type, which stores a tuple of vehicle data

    // Data
    using key_type   = std::pair<int, std::size_t>;
    using value_type = gta3::data_section<main_type, boat_type, bike_type, plane_type, anim_type, final_type>;

    // We have too many get<> in this code, wrapper one of them in another function for sugar
    static data_tuple& get_tuple(final_type& slice)
    { return get<0>(slice); }
    static const data_tuple& get_tuple(const final_type& slice)
    { return get_tuple(const_cast<final_type&>(slice)); }



    // Possible sections
    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info(" ", "%", "!", "$", "^", "\n");  // " " is default and "\n" is a internal thing by us
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + value_type::num_sections, "incompatible sizes");
        return sections.data();
    }





    // Matches for the unique key identifier for a specific data
    struct khash_from_value_visitor : gta3::data_section_visitor<size_t>
    {
        // The main type has the identifier string at the <0>
        size_t operator()(const main_type& slice) const
        { return modloader::hash(get<0>(slice)); }

        // All other types have the string identifier at <1> due the section identifier at <0>
        template<class T>
        size_t operator()(const T& slice) const
        { return modloader::hash(get<1>(slice)); }

        // Anim sections do not have a model name as identifier but a id
        size_t operator()(const anim_type& slice) const
        { return size_t(get<1>(slice)); }

        // So, in the case of a final type we should either get the unique identifier of a main_type or from a anim_type
        size_t operator()(const final_type& slice) const
        {
            return (*this)(*get<0>(get_tuple(slice)));  // forwards to operator()(main_type)
        }

        size_t operator()(const either_blank&) const
        { throw std::invalid_argument("blank type"); }
    };

    static key_type key_from_value(const value_type& value)
    {
        return key_type(value.section()->id, value.apply_visitor(khash_from_value_visitor()));
    }

    // Returns the section pointer for the current line
    static const gta3::section_info* section_by_line(const gta3::section_info* sections, const std::string& line)
    {
        static auto mainsec = gta3::section_info::by_name(sections, " ", -1);
        static auto boatsec = gta3::section_info::by_name(sections, "%", -1);
        static auto bikesec = gta3::section_info::by_name(sections, "!", -1);
        static auto planesec = gta3::section_info::by_name(sections, "$", -1);
        static auto animsec = gta3::section_info::by_name(sections, "^", -1);
        switch(line[0])
        {
            case '%': return boatsec;
            case '!': return bikesec;
            case '$': return planesec;
            case '^': return animsec;
            default:  return mainsec;
        }
    }


    // Runs before merging the data, process this data and build a final_type container.
    // The final_type is important/necessary because it associates a couple of vehicle data (maindata, boatdata, etc)
    // into a single type, meaning all this data is weld together and should be merged together (ohhh, marry me)
    // Don't do this after reading from the file (posread) because of the readme data
    template<class StoreType>
    bool premerge(StoreType& store)
    {
        using container_type = typename StoreType::container_type;
        using iterator = typename container_type::iterator;

        container_type newcontainer;

        // Constructors a functor that matches the section id 'which' from the container key,value pair
        auto fn_match_section = [](int which) -> std::function<bool(const std::pair<key_type, value_type>&)>
        {
            return [=](const std::pair<key_type, value_type>& pair) {
                return pair.first.first == which;
            };
        };

        // Finds a handling with the specific unique identifier in the range [begin, end]
        auto find_handling = [](iterator begin, iterator end, size_t hash)
        {
            return std::find_if(begin, end, [=](const std::pair<key_type, value_type>& pair) {
                return pair.first.second == hash;
            });
        };

        // Finds out the iterators we'll work on... Both the begin and the end point
        auto begin       = store.container().begin();
        auto end         = store.container().end();
        auto main_begin  = begin;
        auto boat_begin  = std::partition_point(main_begin, end, fn_match_section(0));
        auto bike_begin  = std::partition_point(boat_begin, end, fn_match_section(1));
        auto plane_begin = std::partition_point(bike_begin, end, fn_match_section(2));
        auto anim_begin  = std::partition_point(plane_begin, end, fn_match_section(3));
        auto main_end    = boat_begin;
        auto boat_end    = bike_begin;
        auto bike_end    = plane_begin;
        auto plane_end   = anim_begin;
        auto anim_end    = end;

        // Builds an final_type for each main handling config line
        for(auto it = main_begin; it != main_end; ++it)
        {
            static auto finalsec = gta3::section_info::by_name(sections(), "\n");
            auto& main = it->second.get_slice<main_type>();
            auto  hash = it->first.second;  // handling identifier name hashed

            auto boat_it  = find_handling(boat_begin, boat_end, hash);
            auto bike_it  = find_handling(bike_begin, bike_end, hash);
            auto plane_it = find_handling(plane_begin, plane_end, hash);

            // Builds a value_type which contains a piece of final_type
            value_type data(finalsec);
            data.get_slice<final_type>().set<0>(std::make_tuple(
                main_ptr(register_vehdata(main)),
                boat_ptr(boat_it == boat_end? nullptr : register_boatdata(boat_it->second.get_slice<boat_type>())),
                bike_ptr(bike_it == bike_end? nullptr : register_bikedata(bike_it->second.get_slice<bike_type>())),
                plane_ptr(plane_it == plane_end? nullptr : register_planedata(plane_it->second.get_slice<plane_type>()))
             ));

            // Emplace this new set of final data to the new container, for more info about it check the return statement that comes next.
            auto key = key_from_value(data);
            newcontainer.emplace(std::move(key), std::move(data));
        }

        // Anim lines aren't part of a final_type, so add them separately
        for(auto it = anim_begin; it != anim_end; ++it)
            newcontainer.emplace(std::move(*it));

        // Now newcontainer should contain a set of final_type data that references to each other by pointers, 
        // meanwhile the current working container has many split datas that references each other by indice...
        // So replace the working container with the new container, with proper data for datalib analyzes
        store.container() = std::move(newcontainer);
        return true;
    }

    // After merging finish up the shared pointers we own
    template<class StoreType>
    bool posmerge(StoreType&)
    {
        datastore().clear();
        return true;
    }

    // Disables error logging when reading from readme
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        return data_traits::setbyline(store, data, section, line, !reading_from_readme);
    }


    // Stores all the unique data found in all the handling files we have read
    // In the data slices (final_type) we store pointers to the stuff stored on here
    using datastore_t = data_list<main_ptr, boat_ptr, bike_ptr, plane_ptr>;

    // Exposes an static instance of datastore_t
    static datastore_t& datastore()
    {
        static datastore_t ds;
        return ds;
    }

    // Registers the existence of the specified main_type and returns a shared pointer to it.
    static main_ptr register_vehdata(const main_type& a)
    {
        return register_stuff(datastore().get<main_ptr>(), a).first;
    }

    // Registers the existence of the specified boat_type and returns a shared pointer to it.
    static boat_ptr register_boatdata(const boat_type& a)
    {
        return register_stuff(datastore().get<boat_ptr>(), a).first;
    }

    // Registers the existence of the specified bike_type and returns a shared pointer to it.
    static bike_ptr register_bikedata(const bike_type& a)
    {
        return register_stuff(datastore().get<bike_ptr>(), a).first;
    }

    // Registers the existence of the specified plane_type and returns a shared pointer to it.
    static plane_ptr register_planedata(const plane_type& a)
    {
        return register_stuff(datastore().get<plane_ptr>(), a).first;
    }

    // Helper function to register an item into the specific data map.
    // If the item already exists in the map returns a shared pointer to it, otherwise add and return the pointer.
    template<class T>
    static std::pair<std::shared_ptr<T>, bool> register_stuff(std::list<std::shared_ptr<T>>& map, const T& a)
    {
        auto it = std::find_if(map.begin(), map.end(), [&a](const std::shared_ptr<T>& ptr)
        {
            return (*ptr == a);
        });
        if(it == map.end())
            return std::make_pair(*map.emplace(map.end(), std::make_shared<T>(a)), true);
        return std::make_pair(*it, false);
    }
};

/////////////////////// datalib I/O
namespace std
{
    /*
     *  Output for final type slice at handling_traits
     */
    template<class CharT, class Traits> inline
    std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const handling_traits::data_tuple& data_tuple)
    {
        std::string text, tmp;
        text.reserve(256); tmp.reserve(256);

        auto& main = get<0>(data_tuple);
        auto& boat = get<1>(data_tuple);
        auto& bike = get<2>(data_tuple);
        auto& plane = get<3>(data_tuple);
        if(main->get(text))
        {
            if(boat && boat->get(tmp))
                text.append("\n").append(tmp);
            if(bike && bike->get(tmp))
                text.append("\n").append(tmp);
            if(plane && plane->get(tmp))
                text.append("\n").append(tmp);

            os << text;
        }
        else
            os.setstate(std::ios::failbit);

        return os;
    }
}


//
using handling_store = gta3::data_store<handling_traits, std::map<
                        handling_traits::key_type, handling_traits::value_type
                        >>;

REGISTER_RTTI_FOR_ANY(handling_store);


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const handling_traits::value_type&)
        {
            return handling_traits::sections();
        }
    }
}


// Handling Merger
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadHandling = []
    {
        void* handling_data = memory_pointer(0xC2B9C8).get();
        injector::thiscall<void(void*)>::call<0x5BD830>(handling_data);
    };

    // Handling Merger
    plugin_ptr->AddMerger<handling_store>("handling.cfg", true, false, false, reinstall_since_start, gdir_refresh(ReloadHandling));

    // Readme Reader for handling.cfg lines
    plugin_ptr->AddReader<handling_store>([](const std::string& line) -> maybe_readable<handling_store>
    {
        handling_store store;
        reading_from_readme = true;
        if(store.insert(handling_traits::section_by_line(handling_traits::sections(), line), line))
        {
            reading_from_readme = false;
            return store;
        }
        reading_from_readme = false;
        return nothing;
    });
});
