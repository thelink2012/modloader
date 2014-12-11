/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <algorithm>
#include <datalib/detail/flat_linear_map.hpp>

namespace datalib {

// If the key do not exist in at least one custom trait BUT exists in the default trait, just remove the value (return null)
static const int flag_RemoveIfNotExistInOneCustomButInDefault   = 0x0001; 
// If the key do not exist in any of the custom traits, remove the value (return null)
static const int flag_RemoveIfNotExistInAnyCustom               = 0x0002;


/*
 *  find_dominant_data
 *      This algorithm finds the dominant element at a specific key in a bunch of storers objects
 *      Returns a pointer to the found dominant element or nullptr if none found.
 *
 *      first, last -> The range of storers to search the key in
 *      key         -> The key to be searched in each storer, to determine which storer contains the dominant value
 *      flags       -> Flags to act in the search as conditions
 *      
 *      The dominant element is based on a serie of rules:
 *          [*] Elements at custom storers (where 'storer::is_default == false') are dominant over default storers
 *          [*] The less common the element is, the more dominant it is
 *          [*] Elements different from the element at the default store are more dominant
 *          [*] When two or more custom storers have elements of the same dominance weight (i.e. differents), the element returned is from the store nearest to the 'first' iterator.
 *
 *          When the flag_RemoveIfNotExistInOneCustomButInDefault is in effect:
 *              [*] If the element is not present in at least one custom store but is present in the default store, nullptr is returned.
 *          When the flag_RemoveIfNotExistInAnyCustom is in effect:
 *              [*] If the element is not present in any custom store, nullptr is returned.
 *
 *
 */
template<typename Key, typename ForwardIterator> inline
auto find_dominant_data(ForwardIterator first, ForwardIterator last, const Key& key, int flags) ->
    typename std::decay<decltype(*first)>::type::mapped_type*   // CXX14 HELP-ME
{
    using store_type  = typename std::decay<decltype(*first)>::type;
    using key_type    = typename store_type::key_type;
    using mapped_type = typename store_type::mapped_type; 

    //static_assert(std::is_same<key_type, typename std::decay<Key>::type>::value, "type of 'key' parameter is not the same as store_type::key_type");

    // must store the key-value pairs in the same order as they were inserted because of the rule of this algorithm
    // that if many different elements are present the first found is returned
    using map_counter_type = flat_linear_map<
                                std::reference_wrapper<mapped_type>,    // ref to the element; must be a ref because of the implicit conversion into 'type&'
                                std::pair<uint32_t, bool>,              // .first is quantity, .second tells whether the element is present in the default store
                                std::equal_to<mapped_type>              // pred for the key, remember about the implicit conversion?
                               >; 

    map_counter_type counter;
    bool has_any_custom         = false;    // Whether there's any element from a custom store in the first-last range of storers
    bool key_isnt_in_one_custom = false;    // Whether the element has not been found in at least one custom storer
    bool key_is_in_default      = false;    // Whether the element is present in the default storer

    for(auto st = first; st != last; ++st)
    {
        bool is_default = st->default();

        if(st->ready())
        {
            has_any_custom |= !is_default;

            auto& map = st->container();
            auto kv = map.find(key);
            if(kv != map.end())
            {
                auto& count = counter[std::ref(kv->second)];
                ++count.first;
                count.second |= is_default;
                key_is_in_default |= is_default;
            }
            else
            {
                if(is_default == false)
                {
                    key_isnt_in_one_custom = true;
                    if(flags & flag_RemoveIfNotExistInAnyCustom)
                        return nullptr;
                }
            }
        }
    }

    if(counter.size())
    {
        if(flags & flag_RemoveIfNotExistInOneCustomButInDefault)
        {
            if(has_any_custom)
            {
                if(key_is_in_default && key_isnt_in_one_custom)
                    return nullptr;
            }
        }

        // Find the dominant based on the information gathered in the iteration over there
        // Remember the 'if all elements are dominant, the first one is returned'? well, std::min_element has this rule too :)
        auto dom = std::min_element(counter.begin(), counter.end(), [](const map_counter_type::value_type& a, const map_counter_type::value_type& b)
        {
            // Because the dominant element cannot be the default one, returns 'a' if 'b' is default and returns 'b' if 'a' is the default.
            // Otherwise returns the less commmon element, which should be the dominant.
            auto &av = a.second, &bv = b.second;
            return ((av.second || bv.second)? bv.second : (av.first < bv.first));
        });

        return (dom != counter.end()? &dom->first.get() : nullptr);
    }

    return nullptr;
}

template<int Flags>
struct domflags_fn
{
    template<class Key>
    int operator()(const Key&) const { return Flags; }
};


} // namespace datalib
