/*
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#pragma once
#include <utf8.h>
#include <boost/detail/endian.hpp>

namespace unicode
{
    using utf8::exception;
    using utf8::invalid_code_point;
    using utf8::invalid_utf8;
    using utf8::invalid_utf16;
    using utf8::not_enough_room;

    namespace detail 
    {
        inline uint16_t byteswap16(uint16_t val)
        {
            return ((val >> 8) & 0x00FF) | ((val << 8) & 0xFF00);
        }

        inline uint32_t byteswap32(uint32_t val)
        {
            return ((val >> 24) & 0x000000FF) |
                   ((val << 8)  & 0x00FF0000) |
                   ((val >> 8)  & 0x0000FF00) |
                   ((val << 24) & 0xFF000000);
        }

        template< class InputIt1, class InputIt2 >
        bool equal(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
        {
            for(; first1 != last1 && first2 != last2;)
            {
                if(*first1++ != *first2++)
                    return false;
            }
            return first1 == last1 && first2 == last2;
        }
    }

    /// Unicode encodings.
    enum class encoding
    {
        utf8,
        utf16le, utf16be,
        utf32le, utf32be,
    };

    /// Detects the encoding of the text in first-last.
    inline encoding detect_encoding(const void* start_, const void* end_)
    {
        // If the string begins with a BOM, the encoding can be extracted from the BOM.
        static const uint8_t utf8_bom[] = { 0xEF, 0xBB, 0xBF };
        static const uint8_t utf16be_bom[] = { 0xFE, 0xFF };
        static const uint8_t utf16le_bom[] = { 0xFF, 0xFE };
        static const uint8_t utf32be_bom[] = { 0x00, 0x00, 0xFE, 0xFF };
        static const uint8_t utf32le_bom[] = { 0xFF, 0xFE, 0x00, 0x00 };

        auto first = (const uint8_t*)(start_);
        auto last = (const uint8_t*)(end_);

        size_t size = std::distance(first, last);

        // Check for UTF8 BOM.
        if(size >= sizeof(utf8_bom))
        {
            if(std::equal(std::begin(utf8_bom), std::end(utf8_bom), first))
                return encoding::utf8;
        }

        // There is a problem with UTF-16-BE and UTF-32-LE: UTF-32-LE BOM starts with the UTF-16-LE BOM.
        // So check for UTF32 BOMs first!!!
        if(size >= sizeof(utf32be_bom))
        {
            if(std::equal(std::begin(utf32le_bom), std::end(utf32le_bom), first))
            {
                // FIXME: it still can be a UTF-16 string, "<BOM> <NUL>"!
                return encoding::utf32le;
            }
            else if(std::equal(std::begin(utf32be_bom), std::end(utf32be_bom), first))
                return encoding::utf32be;
        }

        // Check for UTF16 BOMs
        if(size >= sizeof(utf16be_bom))
        {
            if(std::equal(std::begin(utf16le_bom), std::end(utf16le_bom), first))
                return encoding::utf16le;
            else if(std::equal(std::begin(utf16be_bom), std::end(utf16be_bom), first))
                return encoding::utf16be;
        }

        // No BOM, let's assume UTF8 for now.
        return encoding::utf8;
    }

    namespace unchecked
    {
        /// Converts from any encoding (`start`-`end`) to UTF-8 (into `result`)
        template<typename Output8It>
        inline Output8It any_to_utf8(encoding format, const void* start_, const void* end_, Output8It result)
        {
#           if !defined(BOOST_ENDIAN_LITTLE_BYTE)
                // This function assumes that we don't have to byteswap LE encodings.
#               error This function currently assumes to be running in a little endian machine for performance reasons.
#           endif
            
            switch(format)
            {
                case encoding::utf8:
                {
                    auto start = (const uint8_t*)(start_);
                    auto end   = (const uint8_t*)(end_);
                    return std::copy(start, end, result);
                }

                case encoding::utf16le:
                {
                    auto start = (const uint16_t*)(start_);
                    auto end   = (const uint16_t*)(end_);
                    return utf8::utf16to8(start, end, result);
                }

                case encoding::utf16be:
                {
                    auto start = (const uint16_t*)(start_);
                    auto end   = (const uint16_t*)(end_);

                    std::vector<uint16_t> convert;
                    convert.reserve(std::distance(start, end));
                    std::transform(start, end, std::back_inserter(convert), detail::byteswap16);

                    return utf8::utf16to8(convert.begin(), convert.end(), result);
                }

                case encoding::utf32le:
                {
                    auto start = (const uint32_t*)(start_);
                    auto end   = (const uint32_t*)(end_);
                    return utf8::utf32to8(start, end, result);
                }

                case encoding::utf32be:
                {
                    auto start = (const uint32_t*)(start_);
                    auto end   = (const uint32_t*)(end_);

                    std::vector<uint32_t> convert;
                    convert.reserve(std::distance(start, end));
                    std::transform(start, end, std::back_inserter(convert), detail::byteswap32);

                    return utf8::utf32to8(convert.begin(), convert.end(), result);
                }

                default:
                {
                    throw std::runtime_error("This should never happen");
                }
            }
        }

        /// Converts from the detected encoding (`start`-`end`) to UTF-8 (into `result`)
        template<typename OutputIt>
        inline OutputIt any_to_utf8(const void* start, const void* end, OutputIt result)
        {
            return any_to_utf8(detect_encoding(start, end), start, end, result);
        }
    }


#if 0
    void test()
    {
        const uint8_t*  u8 = (const uint8_t*) "\xef\xbb\xbf\x74\x65\x73\x74\x31\x32\x33\x00"; 
        const uint16_t* u16_le = (const uint16_t*) "\xff\xfe\x74\x00\x65\x00\x73\x00\x74\x00\x31\x00\x32\x00\x33\x00\x00\x00";
        const uint16_t* u16_be = (const uint16_t*) "\xfe\xff\x00\x74\x00\x65\x00\x73\x00\x74\x00\x31\x00\x32\x00\x33\x00\x00";

        std::vector<uint16_t> utf16line;
        std::vector<uint8_t> utf8line;

        unicode::unchecked::any_to_utf8(u8 + 0, u8 + 11, std::back_inserter(utf8line));
        assert(unicode::detail::equal(utf8line.begin(), utf8line.end(), u8, u8 + 11));
        utf8line.clear();

        unicode::unchecked::any_to_utf8(u16_le + 0, u16_le + 9, std::back_inserter(utf8line));
        assert(unicode::detail::equal(utf8line.begin(), utf8line.end(), u8, u8 + 11));
        utf8line.clear();

        unicode::unchecked::any_to_utf8(u16_be + 0, u16_be + 9, std::back_inserter(utf8line));
        assert(unicode::detail::equal(utf8line.begin(), utf8line.end(), u8, u8 + 11));
        utf8line.clear();
    }
#endif

}
