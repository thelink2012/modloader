/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <type_traits>
#include <datalib/detail/mpl/is_sorted_container.hpp>

namespace datalib {

/*
 *  data_store
 *      Stores a entire data set that composes one data file.
 *      Each piece of data in the set must be a key-value pair.
 *
 *      This object stores it's content in a container of type 'ContainerType', it must meet a std::map like interface, the ordering is user-defined.
 *      A typical example of a ContainerType would be 'std::map<int, data_slice<int, float, float, int, std::string, ...>>'
 *
 *      If ContainerType is a user-defined-type and it store it's element in a sorted manner, a datalib::is_sorted_container specialization
 *      must be made which returns true.
 *
 */
template<typename ContainerType>
class data_store
{
    public:
        // Aliases
        using container_type = ContainerType;
        using key_type       = typename container_type::key_type;
        using mapped_type    = typename container_type::mapped_type;
        using pair_type      = typename container_type::value_type;

        // Is the container_type a sorted container type? Let's find it out.
        static const bool is_sorted = is_sorted_container<container_type>::value;

    protected:
        bool is_ready   = false;    // Has something been readen into this store?
        bool is_default = false;    // Is this a default store (i.e. default content, not custom content, see dominance.hpp for details)
        container_type map;         // The container which stores all the data

    public:

        //
        //  Constructors
        //

        data_store() = default;

        data_store(const data_store& rhs) :
            is_ready(rhs.is_ready), is_default(rhs.is_default), map(rhs.map)
        {}

        data_store(data_store&& rhs) :
            is_ready(rhs.is_ready), is_default(rhs.is_default), map(std::move(rhs.map))
        { rhs.is_ready = false; }

        //
        //  Assignment Operators
        //

        data_store& operator=(const data_store& rhs)
        {
            this->is_ready      = rhs.is_ready;
            this->is_default    = rhs.is_default;
            this->map           = rhs.map;
            return *this;
        }

        data_store& operator=(data_store&& rhs)
        {
            this->is_ready      = rhs.is_ready;
            this->is_default    = rhs.is_default;
            this->map           = std::move(rhs.map);
            rhs.is_ready        = false;
            return *this;
        }

        // Gets the reference to the container used to store the data set
        container_type& container()             { return this->map; }
        const container_type& container() const { return this->map; }

        // Loads content into this store, making it ready. If it was already ready, no work is performed.
        // The content is loaded by a user-defined functor, parser, which can send one argument to itself.
        // Must be static because of derived classes, which needs to specify their actual type ('StoreType')
        template<class StoreType, typename Arg1, class FuncT>
        static bool load(StoreType& store, Arg1&& arg1, FuncT parser)
        {
            if(!store.is_ready) store.is_ready = parser(store, std::forward<Arg1>(arg1));
            return store.is_ready;
        }

        // Clears the content of this store, making it non-ready
        void clear()
        {
            this->map.clear();
            this->is_ready = false;
        }

        // Marks this store as a default store (i.e. default content, not custom content, see dominance.hpp for details)
        void set_as_default(bool as_default = true)
        {
            this->is_default = as_default;
        }

        // Checks the state of this object
        bool ready() const   { return this->is_ready; }
        bool default() const { return this->is_default; }


    public:
        template<typename Result, typename Key, typename ForwardIterator>
        friend Result find_dominant_data(ForwardIterator, ForwardIterator, const Key&, int);
};


} // namespace datalib
