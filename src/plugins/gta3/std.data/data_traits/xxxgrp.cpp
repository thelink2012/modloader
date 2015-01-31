/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::set;
using std::string;

namespace datalib
{
    template<>
    struct data_info<udata<modelname>> : data_info<modelname>
    {
        static const bool ignore = true;
    };
}


// Traits for pedgrp.dat and cargrp.dat
struct xxxgrp_traits : public data_traits
{
    static const bool has_sections      = false;
    static const bool per_line_section  = false;
           
    //
    using key_type   = std::pair<int, size_t>; // grpindex, model_hash
    using value_type = data_slice<either<set<modelname>, udata<modelname>>>;

    key_type key_from_value(const value_type&)
    {
        return std::make_pair(grpindex++, 0);
    }


    // Before the merging process transform the set of models into individual models in each key of the container
    // This allows merging of individual entries in a group not just the entire group container itself
    template<class StoreType>
    static bool premerge(StoreType& store)
    {
        StoreType::container_type newcontainer;

        // Builds the new container, which contains a single model instead of the set of models
        std::for_each(store.container().begin(), store.container().end(), [&](StoreType::pair_type& pair)
        {
            auto& set = *get<std::set<modelname>>(&pair.second.get<0>());
            for(auto& model : set)
            {
                newcontainer.emplace(
                    key_type(pair.first.first, hash_model(model)),
                    value_type(make_udata<modelname>(model)));
            }
        });

        store.container() = std::move(newcontainer);
        return true;
    }

    // Now, before writing the content to the merged file we should reverse the transformation we did in premerge.
    // So this time we should take each model with the same group in the key and put in a set
    template<class StoreType, class MergedList, class FuncDoWrite>
    static bool prewrite(MergedList list, FuncDoWrite dowrite)
    {
        std::map<key_type, value_type> grp_content;

        std::for_each(list.begin(), list.end(), [&](MergedList::value_type& pair)
        {
            auto& model = get(*get<udata<modelname>>(&pair.second.get().get<0>()));
            auto key = key_type(pair.first.get().first, 0);

            // If the key still doesn't exist, make it to be have it's mapped type to be a set of models
            if(grp_content.count(key) == 0)
            {
                grp_content.emplace(key, value_type(set<modelname>()));
            }

            get<set<modelname>>(&grp_content[key].get<0>())->emplace(model);
        });

        list.clear();
        for(auto& x : grp_content)
           list.emplace_back(std::cref(x.first), std::ref(x.second));

        return dowrite(list);
    }

    public: // traits data

        int grpindex = 0;   // Line index we are going tho for groups

        template<class Archive>
        void serialize(Archive& archive)
        { archive(this->grpindex); }
};

struct cargrp_traits : public xxxgrp_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "car groups"; }
        static const char* datafile()   { return "cargrp.dat"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5BD1BB, dtraits>;
};

struct pedgrp_traits : public xxxgrp_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "ped groups"; }
        static const char* datafile()   { return "pedgrp.dat"; }
    };
    
    using detour_type = modloader::OpenFileDetour<0x5BCFFB, dtraits>;
};


template<class Traits>
using xxxgrp_store = gta3::data_store<Traits, std::map<
                        typename Traits::key_type, typename Traits::value_type
                        >>;


template<class Traits>
static void initialise(DataPlugin* plugin_ptr, std::function<void()> refresher)
{
    using store_type = xxxgrp_store<Traits>;
    plugin_ptr->AddMerger<store_type>(Traits::dtraits::datafile(), true, false, false, reinstall_since_load, refresher);
}

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    initialise<pedgrp_traits>(plugin_ptr, gdir_refresh(injector::cstd<void()>::call<0x5BCFE0>));
    initialise<cargrp_traits>(plugin_ptr, gdir_refresh(injector::cstd<void()>::call<0x5BD1A0>));
});
