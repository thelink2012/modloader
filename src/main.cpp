/* 
 *  Testing for DataTraits, by Denilson das Mercês Amorim <dma_2012@hotmail.com>
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "DataTraits.hpp"
#include <iostream>
#include <cstdlib>  /* for rand */
#include <ctime>    /* for time(), to use with srand() */

using namespace DataTraitsNamespace;

static const int times = 10;
static const int nTraits = 5;
static const int maxRand = 5;

struct SData
{
    typedef int key_type;

    int key;
    int i;
    
    bool operator==(const SData& b)
    {
        return (key == b.key && i == b.i);
    }
};

struct MyTraits : public DataTraits<SData>
{
    static bool Parser(const char* filename, DataTraits::container_type& out)
    {
        /* THIS IS A TEST, IGNORE FILENAME AND GET RANDOM DATA */
        for(int i = 0; i < times; ++i)
        {
            auto& data = out[i];
            data.key = i;
            data.i = rand() % maxRand;
        }
        return true;
    }
    
    bool LoadData(const char* filename = 0)
    { return DataTraits::LoadData(filename, Parser); }
};




int main()
{
    srand(time(0));

    std::array<MyTraits, nTraits>   arr;
    auto& a = arr;
    
    for(auto& x : arr)
    {
        x.LoadData("filename");
    }
    
    auto& defaultTrait = a[2];
    defaultTrait.isDefault = true;     /* Default trait (Arquivo original) */
        
#if 1       /* Test when there's removed element */
    defaultTrait.map.erase(4);  /* remove key 4 from default trait.
                                 * This should not make any difference at all for the algorithm */
    
    a[3].map.erase(5);  /* remove key 5 from trait 3 */
    a[4].map.erase(1);  /* remove key 1 from trait 4 */
#endif
    
    std::cout << "=============== TRAITS INFO =====================\n";
    for(auto& x : arr)
    {
        std::cout << "\nTraits data:\n";
        std::cout << "\tIs default: " << x.isDefault << "\n";
        std::cout << "\tIs ready: " << x.isReady << "\n";
        for(auto& pair : x.map)
        {
            auto& v = pair.second;
            std::cout << "\t\t[" << v.key << "]" << " = " << v.i << "\n";
        }
    }
    std::cout << "\n";
    
    int key = rand() % times;
    auto* dominant = FindDominantData<MyTraits>(key, arr.begin(), arr.end());
    
    if(dominant)
    { 
        if(dominant->key != key) std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ DOMINANT KEY IS NOT KEY\n";
        if(dominant->i == defaultTrait.map[key].i) std:: cout << "@@@@@@@@@@@ dominant is default\n";
        std::cout << "\nThe dominant data at key " << key << " is " << dominant->i << "\n";
    }
    else
    {
        std::cout << "No dominant data found at key " << key << "\n";
    }
    
    std::cout << std::endl;
}

