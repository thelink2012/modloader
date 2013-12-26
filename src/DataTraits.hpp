/* 
 *  Created by Denilson das Mercês Amorim <dma_2012@hotmail.com>
 * 
 *  I really don't know how to describe this header.
 *  Basically it provides data structures that finds a 'dominant' element in a serie of structures.
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef ML_DATATRAITS_HPP
#define	ML_DATATRAITS_HPP

#include <algorithm>
#include <ordered_map.hpp>

namespace DataTraitsNamespace   /* well... ugly name but we expect you to do 'using namespace' or alias the name */
{

/* Algorithm flags */

 /*
  * Important: If you add more flags or something be sure to update all the checks on those flags on modloader source code
  *            Do a find (CTRL+F, grep, whatever) and you might find it
  */

// If the key do not exist in at least one custom trait BUT exists in the default trait, just remove the value (return null)
static const int flag_RemoveIfNotExistInOneCustomButInDefault = 1; 
// If the key do not exist in any of the custom traits, remove the value (return null)
static const int flag_RemoveIfNotExistInAnyCustom = 2;

    
/* The following data structure represent a package of values (data) */
//template<class T>       /* T must provide operator== and a typedef key_type */
template<class ContainerType>
struct DataTraits
{
    typedef ContainerType container_type;
    
    typedef typename container_type::key_type       key_type;
    typedef typename container_type::mapped_type    mapped_type;
    typedef typename container_type::value_type     pair_type;
    typedef std::pair<std::reference_wrapper<key_type>, std::reference_wrapper<mapped_type>>    pair_ref_type;
    
    
    bool isReady;           /* Will probably always be true */
    bool isDefault;         /* Is the default (original) trait, see later on the algorithm what that means */
    container_type map;     /* Store data */
    
    DataTraits() :
        isReady(false), isDefault(false)
    {}
    
    template<class FuncT>   /* bool FuncT(const char* filename, SDataTraits<T>::container_type& out) */
    bool LoadData(const char* filename, FuncT parser)
    {
        if(!isReady) isReady = parser(filename, this->map);
        return isReady;
    }
};

/*
 *  FindDominantData
 *      This algorithm finds the dominant data at a specific key looking in a serie of DataTraits
 *
 *      @key    : The key in each DataTraits to find the dominant data
 *      @begin  : The iterator poiting to the beggining of the sequence of DataTraits
 *      @end    : The iterator pointing to the end of the sequence of DataTraits
 *      @return : A pointer (not iterator) to the dominant data; When no data is found, nullptr is returned.
 *
 *      It's important to note "dominant" does not mean the most common here.
 *      The dominant element is based on a serie of rules:
 *          [*] Custom SDataTraits (where SDataTraits::isDefault == false) are dominant over default traits;
 *          [*] When there's a value at the default trait that's not present in a custom trait, the remotion is dominant;
 *          [*] When the value at the custom trait is different from the value at the default trait, the custom trait value is dominant;
 *          [*] When the value at two or more custom traits are different, the most different will be used; Most different means: Not equal to the default value and less common;
 *          [*] When the value at two or more custom traits are different and there's no "most different", the custom trait that gets returned is unspecified.
 *
 */
template<class T, class ForwardIterator> inline
auto FindDominantData(const typename T::key_type& key, ForwardIterator begin, ForwardIterator end, int flags) -> typename T::mapped_type*
{
    /* Object that determines the state of the value in the search */
    struct SCount
    {
        unsigned int num;
        bool isDefault;
        
        SCount() : num(0), isDefault(false)
        {} 
    };

    typedef typename T::mapped_type mapped_type;
    typedef ordered_map<std::reference_wrapper<mapped_type>, SCount, std::equal_to<mapped_type>>    CounterType;
    typedef typename CounterType::value_type    CounterPair;

    bool bExistInDefault = false;
    bool bNotExistInOneCustom = false;
    bool bAnyCustom = false;
    CounterType  count; // Counts number of appearences of a value

    // Iterate on the traits
    for(auto it = begin; it != end; ++it)
    {
        /* Skip this if not initialized */
        if(!it->isReady) continue;

        // Mark if there's any custom in the search
        bAnyCustom |= !it->isDefault;
        
        auto& map  = it->map;       // map of values
        auto  data = map.find(key); // find value based on our key

        if(data == map.end())   // value not found?
        {
            if(!it->isDefault)
            {
                bNotExistInOneCustom = true;
                if(flags & flag_RemoveIfNotExistInAnyCustom)
                    return nullptr;
            }

            // Skip
            continue;
        }

        // Get this iterating value at the 'counter' (or create it if it doesn't exist)
        auto& iCount = count[ std::ref(data->second) ];
        
        // Increase the number of times this value happens
        ++iCount.num; 
        
        // If this value is at the default trait, mark as default
        bExistInDefault |= it->isDefault;
        iCount.isDefault |= bExistInDefault;
    }
    
    // Nothing? uh
    if(!count.size()) { return nullptr; }

    // Apply inst rule if specified
    if(flags & flag_RemoveIfNotExistInOneCustomButInDefault)
    {
        if(bAnyCustom)
        {
            if(bNotExistInOneCustom && bExistInDefault)
                return nullptr;
        }
    }

    // Find the dominant based on the information gathered in the iteration above
    auto xdom = std::min_element(count.begin(), count.end(), [](const CounterPair& a, const CounterPair& b)
    {
        auto &av = a.second, &bv = b.second;
    
        // The default value must be the less dominant (highest element)
        // First of all, see if a or b is default.
        // If b is default, returns true (meaning a < b), otherwise return false (meaning b >= a)
        if(av.isDefault || bv.isDefault)
            return bv.isDefault;
            
        // Then returns whether a is less common than b
        return (av.num < bv.num);
    });
   
    // Assign the found dominant
    if(xdom != count.end())
        return &xdom->first.get();

    return nullptr;
}



}   // namespace


#endif	/* ML_DATATRAITS_HPP */

