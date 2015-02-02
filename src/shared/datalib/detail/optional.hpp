/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#include <boost/optional.hpp>

// Just alias from boost
namespace datalib
{
    using boost::optional;
    using boost::none;
    using boost::make_optional;
};

/*
 *  std::hash specialization for optional objects
 */
namespace std
{
    template<typename T>
    struct hash<datalib::optional<T>>
    {
        std::size_t operator()(const datalib::optional<T>& opt)
        {
            return boost::hash<datalib::optional<T>()(opt);
        }
    };
}
