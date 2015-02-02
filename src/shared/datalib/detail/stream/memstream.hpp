/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <istream>
#include <string>
#include <datalib/detail/stream/fwd.hpp>
#include <datalib/detail/stream/memorybuf.hpp>

namespace datalib {

/*
 *  basic_imemstream
 *      An istream which reads from a memory buffer
 */
template<class CharT, class Traits>
class basic_imemstream : public std::basic_istream<CharT, Traits>
{
    private:
        using base = std::basic_istream<CharT, Traits>;
        using typename base::char_type;

        memorybuf m_rdbuf;

    public:
        explicit basic_imemstream(const std::string& str, std::ios_base::openmode mode = std::ios_base::in) : 
            base(&m_rdbuf), m_rdbuf(str.data(), str.length())
        {}

        explicit basic_imemstream(const void* buf, size_t size,  std::ios_base::openmode mode = std::ios_base::in) : 
            base(&m_rdbuf), m_rdbuf(buf, size)
        {}

        // Returns pointer to the underlying raw string device object. 
        memorybuf* rdbuf()
        { return &m_rdbuf; }

        // too lazy to implement those
        basic_imemstream(const basic_imemstream&) = delete;
        basic_imemstream(basic_imemstream&&) = delete;
        basic_imemstream& operator=(const basic_imemstream&) = delete;
        basic_imemstream& operator=(basic_imemstream&&) = delete;

        // ...
        basic_imemstream& operator>>(short& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(unsigned short& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(int& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(unsigned int& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(long& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(unsigned long& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(long long& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(unsigned long long& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(float& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(double& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(long double& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(bool& value)
        { return base::operator>>(value), *this; }
        basic_imemstream& operator>>(std::ios_base& (*func)(std::ios_base&))
        { return base::operator>>(func), *this; }
        basic_imemstream& operator>>(std::basic_ios<char_type>& (*func)(std::basic_ios<char_type>&))
        { return base::operator>>(func), *this; }
};



} // namespace datalib
