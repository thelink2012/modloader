/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <algorithm>

namespace datalib {

/*
 *  basic_linear_map
 *      An hybrid of a linear container and std::map, elements are stored in the same order as they are inserted (i.e. not sorted as std::map)
 *      and the access of elements are key-value based.
 *
 *      The interface is still incomplete but the most used methods are present.
 *
 */
template<class KeyComp, /* = std::equal_to<Key>*/
         class Container /* = std::list<std::pair<Key, Value>>*/    // notice the Key is non-const, since the container is linear there's no problem is changing the key
>
class basic_linear_map
{
    public:
        // Member types
        using key_compare               = KeyComp;
        using container_type            = Container;
        using allocator_type            = typename container_type::allocator_type;
        using value_type                = typename container_type::value_type;
        using difference_type           = typename container_type::difference_type;
        using size_type                 = typename container_type::size_type;
        using reference                 = typename container_type::reference;
        using const_reference           = typename container_type::const_reference;
        using pointer                   = typename container_type::pointer;
        using const_pointer             = typename container_type::const_pointer;
        using iterator                  = typename container_type::iterator;
        using const_iterator            = typename container_type::const_iterator;
        using reverse_iterator          = typename container_type::reverse_iterator;
        using const_reverse_iterator    = typename container_type::const_reverse_iterator;
        using key_type                  = typename value_type::first_type;                  // notice: non-const
        using mapped_type               = typename value_type::second_type;

    private:
        using result_pair = std::pair<iterator, bool>;
        container_type list;

    public:

        //
        //  Constructors
        //

        basic_linear_map() = default;

        basic_linear_map(const basic_linear_map& rhs)
            : list(rhs.list) {}

        basic_linear_map(basic_linear_map&& rhs)
            : list(std::move(rhs.list)) {}

        //
        //  Assigment Operators
        //

        basic_linear_map& operator=(const basic_linear_map& rhs)
        { this->list = rhs.list; }

        basic_linear_map& operator=(basic_linear_map&& rhs)
        { this->list = std::move(rhs.list); }


        //
        // Observers
        //
        key_compare key_comp() const            { return key_compare(); }
        allocator_type get_allocator() const    { return list.get_allocator(); }

        //
        // Iterators
        //
        iterator begin()                        { return list.begin(); }
        iterator end()                          { return list.end();   }
        const_iterator begin() const            { return list.begin(); }
        const_iterator end() const              { return list.end();   }

        //
        // Capacity
        //
        size_t size() const                     { return list.size();  }
        bool empty() const                      { return list.empty(); }
        size_type max_size() const              { return list.max_size(); }

        //
        // Modifiers
        //

        void clear()
        { return list.clear(); }

        // Finds an element that matches the predicate
        template<class Pred>
        iterator find(Pred pred)
        {
            return std::find_if(begin(), end(), pred);
        }

        // Finds an element with key equivalent to key. 
        iterator find(const key_type& k)
        {
            return find([&](const value_type& pair) {
                return key_compare()(pair.first, k);
            });
        }

        // eturns the number of elements with key key, which is either 1 or 0 since this container does not allow duplicates
        size_type count(const key_type& k) const
        {
            return (this->find(k) == this->end()? 0 : 1);
        }

        // Returns a reference to the value that is mapped to a key equivalent to key, performing an insertion if such key does not already exist.
        mapped_type& operator[](const key_type& k)
        {
            auto it = this->find(k);
            if(it == this->end())
                return this->force_emplace(std::piecewise_construct, std::forward_as_tuple(k), std::forward_as_tuple()).first->second;
            return it->second;
        }

        // Returns a reference to the mapped value of the element with key equivalent to key.
        // If no such element exists, an exception of type std::out_of_range is thrown
        mapped_type& at(const key_type& k)
        {
            auto it = this->find(k);
            if(it == this->end()) throw std::out_of_range();
            return it->second;
        }

        std::pair<iterator, bool> insert(const value_type& pair)
        {
            auto it = this->find(pair.first);
            return (it == this->end()? force_emplace(pair) : result_pair(it, false));
        }

        // Inserts a new element into the container by constructing it in-place with the given args if there is no element with the key in the container.
        template<class... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            value_type pair(std::forward<Args>(args)...);
            auto it = this->find(pair.first);
            return (it == this->end()? force_emplace(std::move(pair)) : result_pair(it, false));
        }

        void erase(const key_type& key)
        {
            auto it = this->find(key);
            if(it != this->end()) this->list.erase(it);
        }


        //
        //  Comparision
        //
        bool operator==(const basic_linear_map& rhs) const
        { return this->list == rhs.list; }
        bool operator!=(const basic_linear_map& rhs) const
        { return this->list != rhs.list; }
        bool operator<(const basic_linear_map& rhs) const
        { return this->list < rhs.list; }
        bool operator<=(const basic_linear_map& rhs) const
        { return this->list <= rhs.list; }
        bool operator>(const basic_linear_map& rhs) const
        { return this->list > rhs.list; }
        bool operator>=(const basic_linear_map& rhs) const
        { return this->list >= rhs.list; }

    public:
        template<class Archive>
        void serialize(Archive& ar)
        {
            ar(this->list);
        }

    private:
        template<class... Args>
        std::pair<iterator, bool> force_emplace(Args&&... args)
        {
            return result_pair(list.emplace(list.end(), std::forward<Args>(args)...), true);
        }

};

} // namespace datalib
