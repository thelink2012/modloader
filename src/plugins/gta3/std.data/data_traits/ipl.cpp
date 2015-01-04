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
using std::string;

// TODO needs more testing (sections, orders, etc)

//
//  IPL files needs a special handling, everything matters on it, we should take care of the following cases:
//
//      [*] Do not reorder elements, they are indices which might be saved in save games, if you add a new item in between another,
//          we'll break the save indices. Additionally streaming IPLs (bnry) links to LODs in the text IPL by indice too.
//
//      [*] When a element is **changed**, we should take care of it being in the same index as it's original value, such as
//          when the modder changes the position of a object in the IPL.
//          (this could probably be archived by looking if any item changed, and if it did find a similar item that has been added)
//
//


//
struct ipl_traits : gta3::data_traits
{
    static const bool do_stlist         = true;     // WE NEED TO PROCESS THE LIST OF STORES BEFORE LETING IT MERGE!
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = true;     // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = true;     // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "scene"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x5B871A, dtraits>;

    // Dominance Flags
    struct domflags_fn
    {
        template<class Key>
        int operator()(const Key& key) const
        {
            // TODO explain why we do return 'no flags' for inst section
            static auto instsec = gta3::section_info::by_name(sections(), "inst");
            return (key.section() == instsec? 0 : flag_RemoveIfNotExistInOneCustomButInDefault);
        }
    };

    // The following string ignorer will be used in the 'inst' section since the modelname is irrelevant
    // Let's not waste space and all that comparing a irrelevant string
    struct ignore_output_dummy
    {
        static modelname output() { return make_insen_string("_");  }
    };
    using model_ignore = ignore<modelname, ignore_output_dummy>;

    // Section slices
    using path_type = data_slice<>;             // path section is unused in SA
    using mult_type = data_slice<>;             // mult section is not even implemented by the game
    using pick_type = data_slice<int, vec3>;
    using zone_type = data_slice<insen<string>, int, bbox, int, labelname>;
    using jump_type = data_slice<vec3, vec3, vec3, vec3, vec3, int>;
    using inst_type = data_slice<int, model_ignore, int, vec3, quat, int>;  // TODO LOD (last) should not be compared on ==
    using auzo_type = data_slice<insen<string>, int, int, either<bbox, bsphere>>;
    using tcyc_type = data_slice<bbox, int, int, real_t, delimopt, real_t, real_t, real_t>;
    using occl_type = data_slice<vec2, real_t, vec2, real_t, real_t, delimopt, real_t, real_t, int>;
    using cars_type = data_slice<real_t, real_t, real_t, real_t, int, int, int, int, int, int, int, int>;
    using enex_type = data_slice<vec3, real_t, vec2, real_t, vec3, real_t, int, int, insen<string>, int, delimopt, int, int, int>;
    using cull_type = data_slice<vec3, real_t, real_t, real_t, real_t, real_t, real_t, int, either<tuple<vec3, real_t>, int>>;
    using grge_type = data_slice<real_t, real_t, real_t, real_t, real_t, real_t, real_t, real_t, int, int, insen<string>, delimopt, int, int, int>;   
                                                        // grge optional part is for http://gtaforums.com/topic/536465-garage-extender/

      
    //
    using key_type = gta3::data_section<inst_type, cull_type, path_type, grge_type, enex_type, pick_type,
                                        jump_type, tcyc_type, auzo_type, mult_type, cars_type, occl_type, zone_type>;

    //
    using value_type = dummy_value; // TODO should always operator== true, read process_stlist for reason

    static const gta3::section_info* sections()
    {
        // Note: must be in the same order as declared in value_type
        static auto sections = gta3::make_section_info( "inst", "cull", "path", "grge", "enex", "pick",
                                                        "jump", "tcyc", "auzo", "mult", "cars", "occl", "zone");
        static_assert(std::tuple_size<decltype(sections)>::value == 1 + key_type::num_sections, "incompatible sizes");
        return sections.data();
    }

    //
    static value_type value_from_key(const key_type&)
    {
        return dummy_value();
    }


    //
    // IPL stuff should be handled in a special manner, please refer to the comment at the top of this source file.
    // This function will take a list of data stores and output a list containing a single data store with the merged data
    template<class StoreType, class ForwardIterator>
    static auto process_stlist(ForwardIterator st_begin, ForwardIterator st_end)
        -> std::list<typename std::iterator_traits<ForwardIterator>::value_type>
    {
        using store_type = typename std::iterator_traits<ForwardIterator>::value_type;
        using list_type = std::list<store_type>;
        
        //
        //  NOTICE:
        //      The following algorithm is horrible!!!!!!!!!!!!!!!!
        //      It's inefficient and doesn't handle all possible cases, we need to rewrite it over time
        //      when we see all it's problems and such.
        //
        //      I won't even document it, I cannot afford to look at it.
        //
        //      (TODO)
        //
        // TODO NOTICE LOD COMPARISION
        //

        auto st_size = std::distance(st_begin, st_end);

        list_type list;

        if(st_size < 3)
        {
            plugin_ptr->Error("std.data: ipl_traits can only do proper merging when there are three of more files in question.\n"
                               "This is possibily a bug, please report it.");
            list.assign(st_begin, st_end);
            return list;
        }


        // NOTE: we compare value_type not key_type, cool story
        using track_list = std::vector<std::reference_wrapper<store_type::pair_type>>; 
        track_list news, missing;
        int defaultly_missing_count = 0;

        auto unique_elems = [](track_list& list)
        {
            for(auto it = list.begin(); it != list.end(); ++it)
            {
                for(auto it2 = list.begin(); it2 != list.end(); )
                {
                    if(it != it2)
                    {
                        if(it->get().first == it2->get().first)
                        {
                            it2 = list.erase(it2);
                            continue;
                        }
                    }
                    ++it2;
                }
            }
        };

        //
        // something is missing when it's missing only in one or two places
        // something is new when it's missing everywhere
        //

        // basic example for my mind
        //
        // ARENA
        // ARENA2
        // ARENA
        //
        // push missing ARENA
        // push <...> <...> **
        // push news ARENA2
        // push <...> <...> **
        // push missing ARENA
        //

        for(auto it = st_begin; it != st_end; ++it)
        {
            bool is_it_default = it->default();
            for(auto& elem : it->container())
            {
                auto missing_count = 0u;
                for(auto xx = st_begin; xx != st_end; ++xx)
                {
                    if(xx == it) continue;

                    auto& cc = xx->container();
                    if(std::find(cc.begin(), cc.end(), elem) == cc.end())
                        ++missing_count;
                }

                if(missing_count != 0)
                {
                    auto is_news = (missing_count == (st_size - 1));
                    if(is_it_default)
                    {
                        if(is_news)
                        {
                            is_news = false;
                            defaultly_missing_count += missing_count;
                        }
                    }
                    else
                    {
                        if(!is_news && defaultly_missing_count > 0)
                        {
                            is_news = true;
                            --defaultly_missing_count;
                        }
                    }
                    (is_news? news : missing).emplace_back(std::ref(elem));
                }
            }
        }

        using assoc_type = std::list<std::pair<std::reference_wrapper<store_type::pair_type>, std::reference_wrapper<store_type::pair_type>>>;
        assoc_type assocs;

        unique_elems(missing);
        unique_elems(news);

        auto sort_by_section = [](const store_type::pair_type& a, const store_type::pair_type& b)
        {
            return a.first.section()->id < b.first.section()->id;
        };

        // needed to sort for the next operation sakeness (section->id stuff)
        std::stable_sort(missing.begin(), missing.end(), sort_by_section);
        std::stable_sort(news.begin(), news.end(), sort_by_section);

        // There must be association between missing and news
        for(auto it1 = missing.begin(), it2 = news.begin();
            it1 != missing.end() && it2 != news.end();
            )
        {
            if(it1->get().first.section() != it2->get().first.section())
            {
                if(it1->get().first.section()->id < it2->get().first.section()->id)
                    ++it1;  // advance to reach it2 section
                else
                    ++it2;  // advance to reach it1 section
            }
            else
            {
                assocs.emplace_back(*it1, *it2);
                it1 = missing.erase(it1);
                it2 = news.erase(it2);
            }
        }
        // what rests on the missing/news lists will be added naturally in the linear map

        auto& ds = *list.emplace(list.end());
        ds.set_as_default();
        ds.set_as_ready();
        auto& map = ds.container();

        for(auto it = st_begin; it != st_end; ++it)
        {
            for(auto& pair : it->container())
            {
                auto app = std::find_if(assocs.begin(), assocs.end(), [&pair](const assoc_type::value_type& refs)
                {
                    if(pair == refs.first.get()) return true;
                    return false;
                });

                if(app != assocs.end())
                    map.insert(app->second);   // dont emplace/move
                else
                    map.emplace(std::move(pair));
            }
        }

        return list;
    }

};

//
using ipl_store = gta3::data_store<ipl_traits, linear_map<  // order of insertion matters
                        ipl_traits::key_type, ipl_traits::value_type
                        >>;


// sections function specialization
namespace datalib {
    namespace gta3
    {
        inline const section_info* sections(const ipl_traits::key_type&)
        {
            return ipl_traits::sections();
        }
    }
}

// Scene Files Merger
static auto xinit = initializer(std::bind(&DataPlugin::AddMerger<ipl_store>, _1, ipl_merger_name, false, false, true, no_reinstall));

