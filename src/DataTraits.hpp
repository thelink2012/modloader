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

#include <map>
#include <algorithm>

namespace DataTraitsNamespace   /* well... ugly name but we expect you to do 'using namespace' or alias the name */
{

/* Those flag names are temporary, we must provide names not related to modloader and more explained */
static const int flag_dominant_ide = 1;
static const int flag_dominant_ipl = 2;
static const int flag_noname       = 128;   // still has no name

    
/* The following data structure represent a package of values (data) */
//template<class T>       /* T must provide operator== and a typedef key_type */
template<class ContainerType>
struct DataTraits
{
    typedef ContainerType container_type;
    
    typedef typename container_type::key_type       key_type;
    typedef typename container_type::mapped_type    mapped_type;
    typedef typename container_type::value_type     pair_type;
    /*
    struct pair_ref_type
    {
        std::reference_wrapper<typename container_type::value_type>
        std::reference_wrapper<typename container_type::value_type>
    };*/
    
    
    typedef std::pair<std::reference_wrapper<key_type>, std::reference_wrapper<mapped_type>>      pair_ref_type;
    
    
    bool isReady;           /* Will probably always be true */
    bool isDefault;         /* Is the default (original) trait, see later on the algorithm what that means */
    container_type map;     /* Store data */
    
    DataTraits() :
        isReady(false), isDefault(false)
    {}
    
    template<class FuncT>   /* bool FuncT(const char* filename, SDataTraits<T>::container_type& out) */
    bool LoadData(const char* filename, FuncT parser)
    {
        return (isReady = parser(filename, this->map));
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
    /* Object refering to the value_type */
    struct SValue
    {
        typename T::mapped_type* p;      // pointer to an value

        SValue(typename T::mapped_type& value)
            : p(&value)
        {}
        
        bool operator==(const SValue& rhs) const
        { return (*this->p == *rhs.p); }
    };

    /* Object that determines the state of the value in the search */
    struct SCount
    {
        unsigned int num;
        bool isDefault;
        
        SCount() : num(0), isDefault(false)
        {} 
    };

    /*
     *  The following data structure was built because well, the standard do not provide a container for such a thing =/
     *  This data structure stores values as <Key, Value> (just like a std::map), where each Key is unique.
     *  Okay but, why not std::map? I didn't work very well for my purposes, I used it like:
     *      typedef std::map<SValue, SCount, CompType>   CounterType; 
     *      Where CompType was a functional object that returned !(a == b)
     */
    struct ListType
    {
        /* We'll try to build it almost similar to the std::map structure, because I was using a map before (as said above)
         * and I don't want to waste time porting container accessing... */
        
        /* !!! THIS DATA STRUCTURE IS NOT SUITABLE (AND WASN'T BUILT) FOR GENERAL PURPOSES !!! */
        
        /* YUNO template? Templates aren't support in nested scope. */
        typedef SValue Key;
        typedef SCount Value;

        /* Similar to an std::pair<Key, Value> */
        struct value_type
        {
            Key first;
            Value second;
            
            value_type(const Key& k) :
                first(k)
            {}
            
            bool operator==(const Key& rhs)
            { return first == rhs; }
        };
        
        typedef std::vector<value_type> Container;
        typedef typename Container::iterator iterator;
        
        Container list;
        
        iterator begin() { return list.begin(); }
        iterator end()   { return list.end(); }  
        size_t size()    { return list.size(); }
        
        /* This operator[] works almost like an std::map::operator[],
         * it checks if Key exist, if not, create it returning a newly allocated value,
         * otherwise returning the previouslly allocated value */
        Value& operator[](const Key& k)
        {
            auto it = std::find(list.begin(), list.end(), k);
            if(it != list.end()) return it->second;
            list.push_back(k);
            return list.back().second;
        }
    };

    typedef ListType CounterType;
    typedef typename CounterType::value_type CounterPair;

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
            }
            
            // Enable the following behaviour by flags
            if(false)
            {
                // If value not found and this is not the default trait, favour the remotion now!
                if(!it->isDefault) return nullptr;
            }
                
            // Skip
            continue;
        }

        // Get this iterating value at the 'counter' (or create it if it doesn't exist)
        auto& iCount = count[ SValue(data->second) ];
        
        // Increase the number of times this value happens
        ++iCount.num; 
        
        // If this value is at the default trait, mark as default
        bExistInDefault |= it->isDefault;
        iCount.isDefault |= bExistInDefault;
    }
    
    // Nothing? uh
    if(!count.size()) { return nullptr; }

    // Apply IPL rule if specified
    if(flags == flag_dominant_ipl)
    {
        if(bAnyCustom)
        {
            /*
            if(bNotExistInOneCustom && bExistInDefault)
            {
                return nullptr;
            }
            */
        }
    }
    
    if(flags == flag_dominant_ide)
    {
        // Need to do nothing
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
        return xdom->first.p;

    return nullptr;
}



}   // namespace


#endif	/* ML_DATATRAITS_HPP */

