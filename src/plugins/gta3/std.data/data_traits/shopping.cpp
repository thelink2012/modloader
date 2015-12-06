/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;
using std::string;
using std::pair;
using std::tuple;

/*
 *  shopping.dat traits
 *      The parsing work is done manually for it since it's structe is... different in relation to the other data files.
 *      Because of it's difference to the 'common', it is HEAVILY commented...
 */

namespace { // avoid unit conflicts

//
//  Common types
//

// Optional value, can be either it or a dash '-'
template<class T>
using opt = either<T, DashValue>;

// Price section common data
using nametag       = string;                                   // nametag of the item
using stat_modifier = pair<opt<string>, opt<int>>;              // stat modifier from price subsections
using price_tail    = tuple<stat_modifier, stat_modifier, int>; // end of a price subsection line


//
//  Enumerations
//

enum class ShoppingSection : uint8_t {
    Prices, Shops
};

enum class TypeOrItem : uint8_t {
    Type, Item
};

enum class KeyDef : uint8_t {
    Begin, Data, End
};

template<>
struct enum_map<ShoppingSection>
{
    static std::map<string, ShoppingSection>& map()
    {
        static std::map<string, ShoppingSection> xmap = {
            { "prices",   ShoppingSection::Prices }, 
            { "shops",    ShoppingSection::Shops  }, 
        };
        return xmap;
    }
};

template<>
struct enum_map<TypeOrItem>
{
    static std::map<string, TypeOrItem>& map()
    {
        static std::map<string, TypeOrItem> xmap = {
            { "type",   TypeOrItem::Type }, 
            { "item",   TypeOrItem::Item  }, 
        };
        return xmap;
    }
};


//
// Information about the current section (.first = Prices|Shops; .second = hash of subsection name)
//
using section_info = std::pair<maybe<ShoppingSection>, maybe<size_t>>; 

} // anon namespace


// Specialization for data_info
namespace datalib
{
    template<>  // The udata<section_info> should be ignored during the data_slice scan/print
    struct data_info<udata<section_info>> : data_info_base
    {
        static const bool ignore = true;
    };
}


//
//  The shopping.dat traits!!!
//
struct shopping_traits : public data_traits
{
    static const bool has_sections      = false;    // we're going by manual handling of sections on this one
    static const bool per_line_section  = false;

    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what()       { return "shopping data"; }
        static const char* datafile()   { return "shopping.dat";  }
    };

    //
    // Slices as Tuples
    //

    // Price Section Types
    using prices_type0         = tuple<price_tail>;                         // Used by: carmods, food
    using prices_type1         = tuple<int, price_tail>;                    // Used by: weapon
    using prices_type2         = tuple<modelname, int, price_tail>;         // Used by: clothes, haircut
    using prices_type3         = tuple<opt<int>, opt<int>, price_tail>;     // Used by: tattoos
    using prices_type          = tuple<modelname, nametag, either<prices_type3, prices_type2, prices_type1, prices_type0>>;
    using prices_key_type      = size_t; // hash of model

    // Shops Section Types
    using shops_type           = pair<TypeOrItem, insen<string>>;
    using shops_key_type       = pair<TypeOrItem, int>; // .first = Type|Item, .second = line id of value (0=type, 1+=items)

    // Section and Subsection Line Matching
    using uppsect_type         = pair<SectionString, ShoppingSection>;  // section
    using subsect_type         = pair<SectionString, insen<string>>;    // subsections
    

    
    //
    //  Key & Value
    //      The key_type should be as follow so the ordering of everything is fine:
    //          <0> = Prices|Shops;
    //          <1> = Base Section Begin|Data|End;
    //          <2> = Sub Section Name Hash;
    //          <3> = Sub Section Begin|Data|End;
    //          <4> = Sub Section Data
    //
    //      The value_type is built manually too so there's no need to worry about order of types at all in the either<> there!
    //
    using data_key_type = either<prices_key_type, shops_key_type>;
    using key_type      = std::tuple<ShoppingSection, KeyDef, size_t, KeyDef, maybe<data_key_type>>;
    using value_type    = data_slice<either<EndString, uppsect_type, subsect_type, prices_type, shops_type>, udata<section_info>>;

    // Helper to make the key_type from a basic set of data
    // secinfo is the working sections info, keydef is the state of the working section or subsection, key is the data_key_type to apply or nothing if not applicable. 
    template<class T>
    static key_type make_key(const section_info& secinfo, KeyDef keydef, T&& key)
    {
        auto rand_keydef = keydef;   // doesn't matter which KeyDef we use here because of the key comparision will stop before
        if(secinfo.first != nothing)
        {
            if(secinfo.second != nothing)
                return key_type(secinfo.first.get(), KeyDef::Data, secinfo.second.get(), keydef, std::forward<T>(key));
            else
                return key_type(secinfo.first.get(), keydef, 0, rand_keydef, std::forward<T>(key)); // last params kinda of ignored
        }
        throw std::invalid_argument("shopping.dat make_key: invalid secinfo object");
    }

    // Matching for the current value type to build a key for it
    struct key_from_value_visitor : gta3::data_section_visitor<key_type>
    {
        shopping_traits& traits;
        const section_info& secinfo;
        key_from_value_visitor(shopping_traits& traits, const section_info& secinfo)
            : traits(traits), secinfo(secinfo) {}

        key_type operator()(const uppsect_type&) const              // base section
        {  return make_key(secinfo, KeyDef::Begin, nothing); }

        key_type operator()(const subsect_type&) const              // sub section
        {  return make_key(secinfo, KeyDef::Begin, nothing); }

        key_type operator()(const prices_type& price) const         // prices section data (in a sub section already)
        {  return make_key(secinfo, KeyDef::Data, data_key_type(prices_key_type(hash_model(get<0>(price))))); }

        key_type operator()(const shops_type& shop) const           // shops section data  (in a sub section already)
        {  return make_key(secinfo, KeyDef::Data, data_key_type(shops_key_type(shop.first, shop.first == TypeOrItem::Type? 0 : ++traits.current_section_line))); }

        key_type operator()(const EndString&) const                 // end string for working section (secinfo)
        {  return make_key(secinfo, KeyDef::End, nothing); }

        key_type operator()(const either_blank&) const              // never happens
        { throw std::invalid_argument("blank type"); }
    };

    // Finds the key for a value
    key_type key_from_value(const value_type& value)
    {
        return get<0>(value).apply_visitor(key_from_value_visitor(*this, datalib::get(get<1>(value))));
    }

    // Does the manual section handling
    template<class StoreType>
    static bool setbyline(StoreType& store, value_type& data, const gta3::section_info* section, const std::string& line)
    {
        shopping_traits& traits = store.traits();

        if(traits.secinfo.first == nothing)    // Not yet in a section
        { 
            data_slice<SectionString, ShoppingSection> section;
            if(section.set(line))
            {
                ShoppingSection sectype = section.get<1>();
                traits.secinfo.first = sectype;
                traits.secinfo.second = nothing;    // to enter in a subsection yet
                data = value_type(uppsect_type(SectionString::Section, sectype), traits.secinfo_udata());
                return true;
            }
        }
        else if(traits.secinfo.second == nothing) // Not yet in a subsection
        {
            data_slice<either<EndString, subsect_type>> section;
            if(section.set(line))   // match either a "end" string or a subsection entry point
            {
                if(auto* xval = get<EndString>(&section.get<0>()))  // matched the end string?
                {
                    data = value_type(EndString::End, traits.secinfo_udata());
                    traits.secinfo.first = nothing;
                    return true;
                }
                else if(auto* xval = get<subsect_type>(&section.get<0>()))  // matched the subsection entry point?
                {
                    auto sectype = xval->second;  // insensitive string
                    traits.current_section_line = 0;
                    traits.secinfo.second = hash_model(sectype);   // not a model but..... we need case insensitive hash
                    data = value_type(subsect_type(SectionString::Section, std::move(sectype)), traits.secinfo_udata());
                    return true;
                }
            }
        }
        else // Then already in a section and subsection, take data
        {
            data_slice<EndString> endsect;
            if(endsect.set(line))   // matches a "end" string?
            {
                data = value_type(EndString::End,  traits.secinfo_udata());
                traits.secinfo.second = nothing;
                return true;
            }
            else switch(traits.secinfo.first.get())
            {
                //  Basically the same code for both, could be a generic... but meh...

                case ShoppingSection::Prices:
                {
                    data_slice<prices_type> prices;
                    if(prices.check(line) && prices.set(line))
                    {
                        data = value_type(std::move(prices.get<0>()), traits.secinfo_udata());
                        return true;
                    }
                    break;
                }

                case ShoppingSection::Shops:
                {
                    data_slice<shops_type> shops;
                    if(shops.check(line) && shops.set(line))
                    {
                        data = value_type(std::move(shops.get<0>()), traits.secinfo_udata());
                        return true;
                    }
                    break;
                }
            }
        }
        return fail(line);
    }

public:
    section_info secinfo;           // Working section info
    int current_section_line = 0;   // Used by the shop section to compute item index for the key

    // Makes a udata from the current section info
    udata<section_info> secinfo_udata()
    {
        return make_udata<section_info>(this->secinfo);
    }

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(this->secinfo, this->current_section_line);
    }
};



//
//  Merger Initializer
//

using shopping_store = gta3::data_store<shopping_traits, std::map<
                        shopping_traits::key_type, shopping_traits::value_type
                        >>;


template<uintptr_t addr>
using detour_type = modloader::OpenFileDetour<addr, shopping_traits::dtraits>;


static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    //  Even the initializer is different from the other data traits since we have to hook many calls for the same file!!!!!!

    auto ReloadShopping = []
    {
        std::string current_shopping = mem_ptr(0xA9A7D8).get<char>();
        injector::cstd<void()>::call<0x49AE30>();   // CShopping::RemoveLoadedShop
        if(current_shopping.size()) // CShopping::LoadShop
            injector::cstd<void(const char*)>::call<0x49BBE0>(current_shopping.c_str());
    };

    auto datafile = shopping_traits::dtraits::datafile();

    auto GetMergedData = plugin_ptr->BindGetMergedData<shopping_store>(datafile, true, false, false);

    plugin_ptr->AddMerger<shopping_store>(datafile, tag_detour, reinstall_since_load,
                std::forward_as_tuple(
                    detour_type<0x49B6AF>(GetMergedData),   // CShopping::LoadStats
                    detour_type<0x49B93F>(GetMergedData),   // CShopping::LoadPrices
                    detour_type<0x49BC98>(GetMergedData)),  // CShopping::LoadShop
                  ReloadShopping);
});
