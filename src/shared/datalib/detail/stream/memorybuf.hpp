/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <strstream>

namespace datalib {

/*
 *  memorybuf
 *
 *      An streambuf implementation that reads stuff from a memory buffer
 *      Unlike stringstreambuf, this does not make a copy of the memory block but reads it directly.
 *      This allows only input operations (i.e. the memory buffer cannot be modified by the buffering object)
 *
 */
class memorybuf : public std::strstreambuf
{
    public:
        memorybuf() :
            strstreambuf((const char*)nullptr, 0), mbuf(nullptr), msize(0)
        {}

        memorybuf(const void* ptr, std::streamsize size) :
            strstreambuf((const char*)ptr, size), mbuf(ptr), msize(size)
        {}

        memorybuf(const memorybuf&) = delete;
        memorybuf(memorybuf&&) = delete;
        memorybuf& operator=(const memorybuf&) = delete;
        memorybuf& operator=(memorybuf&&) = delete;

        const void* buffer()
        { return mbuf; }

        std::streamsize size() const
        { return msize; }

    private:
        int pcount() const
        { /* nope */ return 0; }

    private:
        const void*     mbuf;
        std::streamsize msize;
};



} // namespace datalib
