/* 
 *  Created by Denilson das Mercês Amorim <dma_2012@hotmail.com>
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef ORDERED_MAP_HPP
#define ORDERED_MAP_HPP

#include <algorithm>
#include <list>


/* This is very basic, needs work */

template<class Key, class Value, class KeyComp = std::equal_to<Key>>
class ordered_map//_temp
{
    public:
        typedef KeyComp                         key_compare;
        typedef Key                             key_type;
        typedef Value                           mapped_type;
        typedef std::pair<Key, Value>           value_type;
        typedef std::list<value_type>           container_type;

        typedef typename container_type::iterator        iterator;
        
        container_type list;
  
    private:
        key_compare CompareKey;
        
    public:
        key_compare key_comp() const    { return CompareKey; }
        iterator begin()                { return list.begin(); }
        iterator end()                  { return list.end();   }
        size_t size()                   { return list.size();  }
        
        iterator find(const key_type& k)
        {
            return std::find_if(begin(), end(), [this,&k](const value_type& pair)
            {
                return this->CompareKey(pair.first, k);
            });
        }
        
        mapped_type& operator[](const key_type& k)
        {
            auto it = find(k);
            if(it == end())
            {
                list.emplace_back(k, mapped_type());
                return list.back().second;
            }
            else
                return it->second;
        }
};

#endif

