/*! \file typeindex.hpp
    \brief Support for serializing type information wrapped in type_index
    \ingroup OtherTypes */
/*
  Copyright (c) 2015, Denilson das Merces Amorim
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of cereal nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL RANDOLPH VOORHIES OR SHANE GRANT BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CEREAL_TYPES_TYPEINDEX_HPP_
#define CEREAL_TYPES_TYPEINDEX_HPP_

#include <cereal/cereal.hpp>
#include <cereal/details/helpers.hpp>
#include <typeinfo>
#include <typeindex>
#include <map>
#include <modloader/util/hash.hpp>  // currently uses a hash to map unique identifiers


namespace cereal
{

//! Registers the runtime information for the specified type so that it's type information can be serialized
#define CEREAL_REGISTER_RTTI(T)     CEREAL_REGISTER_RTTI_WITH_NAME(T, #T)

//! Registers the runtime information for the specied type, giving it a user-defined name,
//! so that it's type information can be serialized
#define CEREAL_REGISTER_RTTI_WITH_NAME(T, NAME)                                 \
    namespace cereal {                                                          \
    namespace typeindex_detail {                                                \
                                                                                \
        template<>                                                              \
        struct binding_rtti<T> : std::true_type                                 \
        {                                                                       \
            static const TypeIndexBind<T>& bind;                                \
        };                                                                      \
        const TypeIndexBind<T>& binding_rtti<T>::bind =                         \
                                        ::cereal::detail::StaticObject<         \
                                                TypeIndexBind<T>                \
                                        >::getInstance().bind(NAME);            \
    }} /* end namespace */
    

    namespace typeindex_detail
    {
        /*! Performs a mapping between a type_index and a unique id  to identify the type.
            Currently we using a hash to give the 'unique id', we use this hash approach since
            we didn't want to want to touch cereal library internals to add support
            for 'backreferencing a previosly readen std::type_index
            @internal */
        struct TypeMapping
        {
            std::map<std::type_index, uint32_t> type2hash;  //!< Maps between a type_index and a hash
            std::map<uint32_t, std::type_index> hash2type;  //!< Maps between a hash and a type_index

            //! Registers a type_index and it's respective cross-platform name
            TypeMapping& RegisterType(const std::type_index& type, const char* type_name)
            {
                uint32_t hash = modloader::hash(type_name);
                this->type2hash.emplace(type, hash);
                this->hash2type.emplace(hash, type);
                return *this;
            }

            //! Gets a unique id to be saved that is able to uniquely identify the specified type_index
            uint32_t GetIdFromType(const std::type_index& type) const
            {
                auto it = this->type2hash.find(type);
                if(it == this->type2hash.end())
                    throw cereal::Exception("Trying to serialize a unregistered type index. Use CEREAL_REGISTER_RTTI.");
                return it->second;
            }

            //! Gets the type_index related to the specified unique id
            std::type_index GetTypeFromId(size_t hash) const
            {
                auto it = this->hash2type.find(hash);
                if(it == this->hash2type.end())
                    throw cereal::Exception("Trying to unserialize a unregistered type index Use CEREAL_REGISTER_RTTI.");
                return it->second;
            }
        };

        //! @internal
        template<class T>
        struct TypeIndexBind
        {
            //! Binds the type_index related to T with a cross-platform name
            const TypeIndexBind& bind(const char* type_name)
            {
                cereal::detail::StaticObject<TypeMapping>::getInstance().RegisterType(typeid(T), type_name);
                return *this;
            }
        };

        //! @internal
        template<class T>
        struct binding_rtti : std::false_type   // The ::value is used to know if the 
        {};                                     // RTTI for the specified T has been registered

    }   /* typeindex_detail */



    //! Checks whether the specified type runtime information has been bind to cereal in this translation unit
    template<class T>
    struct has_rtti :
        std::integral_constant<bool, typeindex_detail::binding_rtti<T>::value>
    {};

    //! Saving std::type_index
    template<class Archive> inline
    void save(Archive& archive, const std::type_index& type)
    {
        uint32_t id = cereal::detail::StaticObject<typeindex_detail::TypeMapping>::getInstance().GetIdFromType(type);
        archive(_CEREAL_NVP("typeid", id));
    }

    //! Loading std::type_index
    template<class Archive> inline
    void load(Archive& archive, std::type_index& type)
    {
        uint32_t id;
        archive(_CEREAL_NVP("typeid", id));
        type = cereal::detail::StaticObject<typeindex_detail::TypeMapping>::getInstance().GetTypeFromId(id);
    }
    

} // namespace cereal

#endif // CEREAL_TYPES_TYPEINDEX_HPP_
