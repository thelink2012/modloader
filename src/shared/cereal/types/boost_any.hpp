/*! \file boost_any.hpp
    \brief Support for serializing the type erased type boost::any
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
#ifndef CEREAL_TYPES_BOOST_ANY_HPP_
#define CEREAL_TYPES_BOOST_ANY_HPP_

#include <boost/any.hpp>
#include <cereal/cereal.hpp>
#include <cereal/details/helpers.hpp>
#include <cereal/types/typeindex.hpp>
#include <map>
#include <cereal/archives/binary.hpp>   // only binary archives supported, how do we bind to all registered archives?

namespace cereal
{

//! Allows the type T to be serialized from a boost::any object
/*! The type T must have been previosly registered for RTTI using CEREAL_REGISTER_RTTI's
    from 'cereal/types/typeindex.hpp' */
#define CEREAL_REGISTER_FOR_ANY(T)                                                      \
    namespace cereal {                                                                  \
    namespace boost_any_detail {                                                        \
                                                                                        \
        template<>                                                                      \
        struct bind_any<T> {                                                            \
            static const BuildAnyCast<T>& caster;                                       \
        };                                                                              \
        const BuildAnyCast<T>& bind_any<T>::caster =                                    \
                                        ::cereal::detail::StaticObject<                 \
                                                BuildAnyCast<T>                         \
                                        >::getInstance().bind();                        \
    }} /* end namespace */
    

    
    namespace boost_any_detail
    {
        //! @internal see save() for details
        struct AnyWrapper
        {
            AnyWrapper(const boost::any& any) : any(any) {}
            const boost::any& any;
        };

        //! @internal
        struct RttiCasters
        {
            using input_caster  = std::function<void(cereal::BinaryInputArchive&, boost::any&)>;
            using output_caster = std::function<void(cereal::BinaryOutputArchive&, const boost::any&)>;
            
            std::map<std::type_index, input_caster>  input_casters;     //!< Map of casting between the key type and a boost::any
            std::map<std::type_index, output_caster> output_casters;    //!< Map of casting between a boost::any and the key type

            //! Adds a caster between the 'type' and a boost::any
            void AddInputCaster(std::type_index type, input_caster caster)
            {
                input_casters.emplace(std::move(type), std::move(caster));
            }
            
            //! Adds a caster between a boost::any and the 'type'
            void AddOutputCaster(std::type_index type, output_caster caster)
            {
                output_casters.emplace(std::move(type), std::move(caster));
            }

            //! Writes the content of a boost::any into an archive
            template<class Archive>
            bool CastIntoArchive(Archive& archive, const boost::any& any, const std::type_index& type) const
            {
                auto it = this->output_casters.find(type);
                if(it == this->output_casters.end())
                    throw cereal::Exception("Trying to serialize a type in boost::any that hasn't been registered with CEREAL_REGISTER_FOR_ANY");

                it->second(archive, any);
            }

            //! Reads the content of a boost::any from an archive
            template<class Archive>
            bool CastFromArchive(Archive& archive, boost::any& any, const std::type_index& type) const
            {
                auto it = this->input_casters.find(type);
                if(it == this->input_casters.end())
                    throw cereal::Exception("Trying to unserialize a type in boost::any that hasn't been registered with CEREAL_REGISTER_FOR_ANY");

                it->second(archive, any);
            }

        };

        //! @internal
        template<class T>
        struct BuildAnyCast
        {
            //! Binds an caster between the type T and a boost::any (and vice-versa)
            const BuildAnyCast& bind()
            {
                // Maybe this check should be performed at runtime, checking the state of binding_rtti<T>::bind,
                // this would allow the RTTI to be registered in one translation unit and the any registrations
                // in another translation unit.
                static_assert(has_rtti<T>::value,
                    "Missing cereal RTTI information for the type T.\n "
                    "A 'boost::any' serializer requires RTTI information for all possible types it may assume.\n "
                    "Please register RTTI types by using 'CEREAL_REGISTER_RTTI' macro from 'cereal/types/type_info.hpp'. ");

                cereal::detail::StaticObject<RttiCasters>::getInstance().AddInputCaster(typeid(T),
                    [](cereal::BinaryInputArchive& archive, boost::any& any)
                {
                    using Archive = cereal::BinaryInputArchive;
                    T temp;
                    archive(_CEREAL_NVP("data", temp));
                    any = std::move(temp);
                });

                cereal::detail::StaticObject<RttiCasters>::getInstance().AddOutputCaster(typeid(T),
                    [](cereal::BinaryOutputArchive& archive, const boost::any& any)
                {
                    using Archive = cereal::BinaryOutputArchive;
                    archive(_CEREAL_NVP("data", *boost::any_cast<T>(&any)));
                });

                return *this;
            }
        };

        //! @internal
        template<class T>
        struct bind_any {};

    } /* bind_any_detail */

    //! Saving for boost::any
    /*! This method uses an 'AnyWrapper' as argument that can be implicitly constructed
        from a boost::any object, we do need this wrapper otherwise this save method would
        accept every type sent to be saved not just boost::any. */
    template<class Archive> inline
    void save(Archive& archive, const boost_any_detail::AnyWrapper& wrapper)
    {
        const boost::any& any = wrapper.any;

        archive(_CEREAL_NVP("has_anything", !any.empty()));
        if(!any.empty())
        {
            std::type_index type = any.type();
            archive(_CEREAL_NVP("typeid", type));
            cereal::detail::StaticObject<boost_any_detail::RttiCasters>::getInstance().CastIntoArchive(archive, any, type);
        }
    }

    //! Loading for boost::any
    template<class Archive> inline
    void load(Archive& archive, boost::any& any)
    {
        bool anything;

        archive(_CEREAL_NVP("has_anything", anything));
        if(anything)
        {
            std::type_index type = typeid(void);
            archive(_CEREAL_NVP("typeid", type));
            cereal::detail::StaticObject<boost_any_detail::RttiCasters>::getInstance().CastFromArchive(archive, any, type);
        }
        else
            any = boost::any();
    }
    

} // namespace cereal

#endif // CEREAL_TYPES_BOOST_ANY_HPP_
