/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <ios>
#include <cctype>
#include <datalib/detail/stream/fwd.hpp>
#include <datalib/detail/stream/memorybuf.hpp>

namespace datalib {

/*
 *  basic_icheckstream / icheckstream
 *
 *      A dummy stream that checks if the possible conversions performed by an istream are fine.
 *      Does not send anything to anywhere, just checks. If a conversion cannot be perfoemd, failbit is set.
 *      Used as a fast checking way if a conversion can be made without taking time to actually convert.
 *      Use the 'operator>>' to check if a conversion is possible.
 *
 *      Template parameters:
 *          @CharT  -- Type of character to deal with
 *          @Traits -- Character traits
 */
template<typename CharT, typename Traits>
class basic_icheckstream : public virtual std::basic_ios<CharT, Traits>
{
    protected:
        using streambuf_iterator    = std::istreambuf_iterator<CharT, Traits>;
        using base                  = std::basic_ios<CharT, Traits>;
        using ios_base              = base;
        using typename base::pos_type;
        using typename base::off_type;
        using typename base::char_type;
        using typename base::int_type;
        using typename base::traits_type;
        
        std::streamsize m_gcount;       // Number of characters extracted from the last operation
        memorybuf       m_rdbuf;        // Buffering object

    public:

        // Constructs the object 
        basic_icheckstream(const std::string& str) :
            m_gcount(0), m_rdbuf(str.data(), str.length())
        {
            base::init(&m_rdbuf);
        }

        basic_icheckstream(const char* str, size_t size) :
            m_gcount(0), m_rdbuf(str, size)
        {
            base::init(&m_rdbuf);
        }

        // Destructs the object 
        virtual ~basic_icheckstream()
        {}

        // Cannot copy
        basic_icheckstream(const basic_icheckstream&) = delete;
        basic_icheckstream& operator=(const basic_icheckstream& rhs) = delete;
        // Cannot work with a temporary string
        basic_icheckstream(std::string&& str) = delete;

        // Checks if no error has occurred
        explicit operator bool() const
        { return !base::fail(); }

        // Checks if any error has occurred
        bool operator!() const
        { return base::fail(); }

        // Returns number of characters extracted by last unformatted input operation 
        std::streamsize gcount() const
        { return m_gcount; }

        // Returns pointer to the underlying raw string device object. 
        memorybuf* rdbuf()
        { return &m_rdbuf; }
	
        // Sets the input position indicator to absolute (relative to the beginning of the file) value pos
        basic_icheckstream& seekg(pos_type pos)
        {
            this->clear(this->rdstate() & ~std::ios::eofbit);
            sentry xsentry(*this, true);
            if(xsentry)
            {
                if(rdbuf()->pubseekpos(pos, std::ios::in) == pos_type(-1))
                    this->setstate(std::ios::failbit);
            }
            return *this;
        }

        // Sets the input position indicator to position off, relative to position, defined by dir. Specifically, executes 
        basic_icheckstream& seekg(off_type off, std::ios::seekdir dir)
        {
            this->clear(this->rdstate() & ~std::ios::eofbit);
            sentry xsentry(*this, true);
            if(xsentry)
            {
                if(rdbuf()->pubseekoff(off, dir, std::ios::in) == pos_type(-1))
                    this->setstate(std::ios::failbit);
            }
            return *this;
        }

        // Returns input position indicator of the current associated streambuf object. 
        pos_type tellg()
        {
            sentry xsentry(*this, true);
            if(xsentry && this->fail() == false)
                return rdbuf()->pubseekoff(0, std::ios::cur, std::ios::in);
            else
                return pos_type(-1);
        }


        // Integer Checkers
        basic_icheckstream& operator>>(const short& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const unsigned short& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const int& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const unsigned int& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const long& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const unsigned long& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const long long& value)
        { return this->check_integer(); }
        basic_icheckstream& operator>>(const unsigned long long& value)
        { return this->check_integer(); }

        // Floating Point Checkers
        basic_icheckstream& operator>>(const float& value)
        { return this->check_real(); }
        basic_icheckstream& operator>>(const double& value)
        { return this->check_real(); }
        basic_icheckstream& operator>>(const long double& value)
        { return this->check_real(); }

        // Boolean Checkers
        basic_icheckstream& operator>>(const bool& value)
        { return this->check_bool(); }
 	
        // Manipulators, functions which get called to modify the state of the stream
        basic_icheckstream& operator>>(std::ios_base& (*func)(std::ios_base&))
        {
            func(*this);
            return *this;
        }

        // Manipulators, functions which get called to modify the state of the stream
        basic_icheckstream& operator>>(std::basic_ios<char_type>& (*func)(std::basic_ios<char_type>&))
        {
            func(*this);
            return *this;
        }

        // No support for locales!!
        char_type widen(char x) const
        {  return x; }

        // Reads the next piece of string in the stream and puts it in 'out' if 'sideeffect=true'
        //template<class Allocator>
        basic_icheckstream& read_string(std::string& out, bool sideeffect = true);

    public:

        /*
            An object of class sentry is constructed in local scope at the beginning of each member function of an input stream
            that performs input (both formatted and unformatted). Its constructor prepares the input stream: checks if the stream is already in a failed state,
            flushes the tie()'d output streams, skips leading whitespace unless noskipws flag is set, and performs other implementation-defined tasks if necessary.
            All cleanup, if necessary, is performed in the destructor, so that it is guaranteed to happen if exceptions are thrown during input. 
        */
        class sentry
        {
            public:
                // The constructor just calls the private perform_sentry
                sentry(basic_icheckstream& is, bool noskipws = false)
                {
                    this->result = is.perform_sentry(noskipws);
                }

                // Cannot copy
		        sentry(const sentry&) = delete;
		        sentry& operator=(const sentry&) = delete;

                // Checks whether the preparation of the input stream was successful. 
		        explicit operator bool() const
			    { return this->result; }

            private:
                bool result;
        };

        /*
            Reposition the stream on the destructor so it stays in the same place it was when the constructor was called.
            You can manually reposition the stream by calling the repos() member and avoid reposition on destruction by calling norepos()
            Useful on this stream because a reading failure.
            Note this should probably come before the sentry construction.

            NOTE You might ask why to use reposer instead of .tellg() and .seekg()
            Well, because it is faster since it does not go tho a sentry.
        */
        class reposer
        {
            public:
                // Constructs gets currect stream position
                // A dummy reposer does nothing
                reposer(basic_icheckstream& is, bool dummy = false) : is(is), dorepos(true), dummy(dummy)
                {
                    this->pos = is.rdbuf()->pubseekoff(0, std::ios::cur, std::ios::in);
                }

                // Cannot copy
		        reposer(const reposer&) = delete;
		        reposer& operator=(const reposer&) = delete;

                // Destructor reposition the stream where it was in the construction time
                ~reposer()
                {
                    if(!dummy && dorepos) this->repos();
                }

                // Reposition to the position of construction time
                basic_icheckstream& repos()
                {
                    if(!dummy) is.rdbuf()->pubseekpos(this->pos, std::ios::in);
                    return is;
                }

                // Avoid reposition during destruction
                basic_icheckstream& norepos(bool noreposx = true)
                {
                    this->dorepos = !noreposx;
                    return is;
                }

                // Same as calling 'norepos(noreposx)'
                basic_icheckstream& operator()(bool noreposx)
                {
                    return norepos(noreposx);
                }

            private:
                basic_icheckstream& is;
                std::streampos pos;
                bool dorepos, dummy;
        };


    private: // Forwarders from operator>>

        // Forwarded from operators>> with integer values
        basic_icheckstream& check_integer()
        {
            reposer xrepos(*this, true);
            sentry  xsentry(*this);
            if(xsentry)
            {
                return xrepos(!!this->may_match_sign().could_match_base().match_integer_by_base().match_end());
            }
            return *this;
        }

        /// Forwarded from operators>> with floating point values
        basic_icheckstream& check_real()
        {
            reposer xrepos(*this, true);
            sentry  xsentry(*this);
            if(xsentry)
            {
                return xrepos(!!this->match_float().match_end());
            }
            return *this;
        }

        // Forwarded from operator>> with boolean values
        basic_icheckstream& check_bool()
        {
            reposer xrepos(*this, true);
            sentry  xsentry(*this);
            if(xsentry)
            {
                return xrepos(!!this->match_bool().match_end());
            }
            return *this;
        }

    protected: // Matchers

        //
        // Meaning of function namings:
        //  may_    means may or mayn't match depending if it's present
        //  could_  means may or mayn't match depending on stream flags
        //  try_    is the same as may_ but returns a boolean specifying if match has been matched
        // 

        // Tries to match either a '+' or '-' sign.
        bool try_match_sign()
        {
            if(*this)
            {
                auto x = sgetc();
                if(x == '+' || x == '-') return !!skipc(x);
            }
            return false;
        }

        // Tries to match either a '+' or '-' sign
        basic_icheckstream& may_match_sign()
        {
            this->try_match_sign();
            return *this;
        }

        // Could match a base (0x, 0, none) depending on the format flags
        basic_icheckstream& could_match_base()
        {
            int_type c;
            if(*this)
            {
                if(!traits_type::eq_int_type(sgetc(), traits_type::eof()))
                {
                    if(this->flags() & std::ios_base::hex)
                    {
                        if(sbumpc() == '0')
                        {
                            if((c = sbumpc(), c == 'x' || c == 'X'))
                                return *this;
                            if(!traits_type::eq_int_type(c, traits_type::eof()))
                                sungetc();
                        }
                        sungetc();
                    }
                    else if(this->flags() & std::ios_base::oct)
                    {
                        if(sbumpc() == '0') return *this;
                        sungetc();
                    }
                }
            }
            return *this;
        }


        // Finishes a match by checking if the current character is a blank space
        basic_icheckstream& match_end()
        {
            if(*this)
            {
                auto c = sbumpc();
                if(!this->is_separator(c))
                    return this->failed();
            }
            return *this;
        }



        // Matches a boolean 'true'/'false' or '1'/'0', depending on the boolalpha format flag.
        basic_icheckstream& match_bool()
        {
            if(*this)
            {
                if(this->flags() & std::ios_base::boolalpha)
                {
                    // Only accepts 'true' and 'false', not 'TRUE' and 'FALSE'
                    streambuf_iterator it(this->rdbuf());
                    
                    if(*it == 't')
                    {
                        // Must be "true"
                        if(!(*++it == 'r' && *++it == 'u' && *++it == 'e' && (++it, true)))
                            return failed();
                    }
                    else if(*it == 'f')
                    {
                        // Must be "false"
                        if(!(*++it == 'a' && *++it == 'l' && *++it == 's' && *++it == 'e' && (++it, true)))
                            return failed();
                    }
                    else // Hmm, it should've matched something
                        return failed();
                }
                else
                {
                    // Just try '0' and '1' for booleans
                    int_type c = sbumpc();
                    if(!(c == '0' || c == '1')) return failed();
                }
            }
            return *this;
        }

        // Matches a integer using the base set in the showbase format flags
        basic_icheckstream& match_integer_by_base()
        {
            return match_integer([this](int c) {
                    return (this->flags() & std::ios_base::hex? isxdigit :
                            this->flags() & std::ios_base::oct? isodigit :
                            isdigit)(c);
            });
        }

        // Matches a integer assuming base 10
        icheckstream& match_integer()
        {
            return match_integer([](int c) { return isdigit(c); });  // functors can be inlined by most compilers while function pointers cannot
        }

        // Matches a integer assuming the digits characters for this integer return true when passed to (isdigit)
        template<class IsDigitFunctor>
        basic_icheckstream& match_integer(IsDigitFunctor isdigit)
        {
            if(*this)
            {
                int_type c = sgetx();
                if(!traits_type::eq_int_type(c, traits_type::eof()))
                {
                    if(!isdigit(c)) return this->failed();
                    for(c = snextc(); isdigit(c); c = snextc()) {}
                }
            }
            return *this;
        }

        // Matches a real number (supports exponents)
        basic_icheckstream& match_float()
        {
            if(*this)
            {
                bool has_exp = false;       // Has found a exponential base
                int_type c;

                // Tries to match a decimal base
                auto try_match_decimal = [this, &has_exp](int_type c) -> bool
                {
                    if(*this)
                    {
                        if(c == '.')
                        {
                            skipc(c);
                            if(isdigit(sgetc()))
                                return !this->match_integer().fail();
                            return true;
                        }
                    }
                    return false;
                };

                // Tries to match a exponential base
                auto try_match_exponent = [this, &has_exp](int_type c) -> bool
                {
                    if(*this)
                    {
                        if(!has_exp && (c == 'e' || c == 'E'))
                        {
                            has_exp = true;
                            skipc(c);
                            return !this->may_match_sign().match_integer().fail();
                        }
                    }
                    return false;
                };

                // Do the actual matching of the floating point
                if(this->may_match_sign().match_integer())
                {
                    // After the initial integer, we can have either a decimal point or exponent
                    c = sgetc();
                    if(!try_match_decimal(c) && !try_match_exponent(c))
                        return *this;

                    // After a decimal digit we can only have a exponent
                    // We cannot have another decimal after a decimal or after a exponent
                    c = sgetc();
                    if(!try_match_exponent(c))
                        return *this;
                }
            }
            return *this;
        }


    private:    // I/O and Parsing Helpers

        // Performs the preparation for a input operation
        // Always use the RAII sentry class to have access to this function
        bool perform_sentry(bool noskipws)
        {
            if(this->good())
            {
                // Skip whitespaces if necessary
                if(!noskipws && (this->flags() & ios_base::skipws))
                {
                    for(auto c = sgetc(); ; c = snextc())
                    {
                        if(traits_type::eq_int_type(c, traits_type::eof()))
                        {
                            this->setstate(ios_base::failbit | ios_base::eofbit);
                            break;
                        }
                        else if(!isspace(c))
                            break;
                    }
      
                    if(this->good()) return true;
                }
                else
                {
                    // Completly good, no whitespace skip
                    return true;
                }
            }
            
            // The stream isn't in a good state for input operations...
            this->setstate(ios_base::failbit);
            return false;
        }

        // Called whenever a conversion couldn't be performed. Sets the failure bits and returns itself.
        basic_icheckstream& failed()
        {
            this->setstate(std::ios_base::failbit);
            return *this;
        }

        // Gets the current character on the streambuf without modifying the stream pointer
        int_type sgetc()
        {
            return rdbuf()->sgetc();
        }

        // Gets the current character on the streambuf and then increases the stream pointer
        int_type sbumpc()
        {
            return rdbuf()->sbumpc();
        }

        // Gets the next character on the streambuf and then increases the stream pointer to point to that character
        int_type snextc()
        {
            return rdbuf()->snextc();
        }

        // Moves the next pointer in the input sequence back by one 
        int_type sungetc()
        {
            return rdbuf()->sungetc();
        }




        // Skips the current character assuming it's the same as c
        // Causes failure bits to be set if the current character is not c
        basic_icheckstream& skipc(int_type c)
        {
            if(!traits_type::eq_int_type(sbumpc(), c)) return this->failed();
            return *this;
        }

        // Helper, if the character x is blank, cause failure bits to be set and returns eof.
        int_type xaux(int_type x)
        {
            if(traits_type::eq_int_type(x, traits_type::eof()) || isspace(x))
            {
                this->failed();
                return traits_type::eof();
            }
            return x;
        }

        // Gets the current character on the streambuf and then increases the stream pointer
        // If the captured character is blank, causes failure bits to be set and returns eof
        int_type sbumpx()
        {
            return xaux(sbumpc());
        }

        // Gets the current character on the streambuf without modifying the stream pointer
        // If the captured character is blank, causes failure bits to be set and returns eof
        int_type sgetx()
        {
            return xaux(sgetc());
        }



    public:
        // Fast check for space characters
        static bool isspace(int c)
        {
            return (c == 0x20) || (c >= 0x09 && c <= 0x0D);
        }

        // Fast check for integral digits
        static bool isdigit(int c)
        {
            return c >= '0' && c <= '9';
        }

        // Fast check for hexadecimal digits
        static bool isxdigit(int c)
        {
            return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
        }

        // Fast check for octal digits
        static bool isodigit(int c)
        {
            return c >= '0' && c <= '7';
        }

        // Checks if the character is blank
        static bool is_separator(int_type c)
        {
            return traits_type::eq_int_type(c, traits_type::eof()) || isspace(c);
        }
};


/*
 *  String Input Operators
 */


// Reads string with optional side effects
template<class CharT, class Traits/*, class Allocator*/> inline
basic_icheckstream<CharT, Traits>& basic_icheckstream<CharT, Traits>::read_string(std::string& str, bool sideff)
{
    using icheckstream = basic_icheckstream<CharT, Traits>;

    auto& is = *this;
    auto count = is.width() > 0 && is.width() < str.max_size()? is.width() : str.max_size();
    bool changed = false;

    typename icheckstream::reposer xrepos(is);
    typename icheckstream::sentry  xsentry(is);
    if(xsentry)
    {
        std::ios_base::iostate state = 0;               // State of the stream after the operation
        auto n = count; // no need to (count-1) because std::string does not need a null terminator
        if(sideff) str.erase(); // optional side effect

        // The delimiter character shouldn't get extracted
        auto x = is.rdbuf()->sgetc();
        for(std::streamsize i = 0; i < n; ++i)
        {
            // Check eof first, just in case delim is also eof.
            if(Traits::eq_int_type(x, Traits::eof())) { state |= std::ios_base::eofbit; break; }
            if(icheckstream::is_separator(x)) break;

            // Put this character on 's', add to gcount() and read next char
            if(sideff) str.append(1, Traits::to_char_type(x)); // optional side effect
            changed = true;
            x = is.rdbuf()->snextc();
        }

        if(!changed) state |= std::ios_base::failbit;   // If no character has been extracted, set failbit
        if(state) is.setstate(state);                   // If any bit has been set during this operation, set it
    }

    is.width(0);                    // Clear width field
    return xrepos(!!is);
}

// Reads one char
template<class CharT, class Traits> inline
basic_icheckstream<CharT, Traits>& operator>>(basic_icheckstream<CharT, Traits>& is, const CharT& ch)
{
    using icheckstream = basic_icheckstream<CharT, Traits>;

    typename icheckstream::reposer xrepos(is);
    typename icheckstream::sentry  xsentry(is);
    if(xsentry)
    {
        auto x = is.rdbuf()->sbumpc();
        if(Traits::eq_int_type(x, Traits::eof()))
            is.setstate(std::ios::failbit | std::ios::eofbit);
        else if(!icheckstream::is_separator(is.rdbuf()->sgetc()))   // next char must be a separator
            is.setstate(std::ios::failbit);
        //else
        //  ch = x; -- check stream do not change states
    }

    return xrepos(!!is);
}

// Reads width() characters
template<class CharT, class Traits> inline
basic_icheckstream<CharT, Traits>& operator>>(basic_icheckstream<CharT, Traits>& is, const CharT* s)
{
    using icheckstream = basic_icheckstream<CharT, Traits>;

    auto count = is.width();
    bool changed = false;

    typename icheckstream::reposer xrepos(is);
    typename icheckstream::sentry  xsentry(is);
    if(xsentry)
    {
        std::ios_base::iostate state = 0;               // State of the stream after the operation
        auto n = count - 1;

        if(count > 1)   // If has space only for one character, extract nothing but puts a null terminator on 's'
        {
            // The delimiter character shouldn't get extracted
            auto x = is.rdbuf()->sgetc();
            for(std::streamsize i = 0; i < n; ++i)
            {
                // Check eof first, just in case delim is also eof.
                if(Traits::eq_int_type(x, Traits::eof())) { state |= std::ios_base::eofbit; break; }
                if(icheckstream::is_separator(x)) break;

                // Put this character on 's', add to gcount() and read next char
                //*s++ = Traits::to_char_type(x);   - don't touch the memory (stream just checks)
                changed = true;
                x = is.rdbuf()->snextc();
            }
        }

        if(!changed) state |= std::ios_base::failbit;   // If no character has been extracted, set failbit
        if(state) is.setstate(state);                   // If any bit has been set during this operation, set it
    }

    //if(count) *s++ = CharT();     // If 's' has enought space, put a null terminator on it    - don't touch the memory (stream just checks)
    is.width(0);                    // Clear width field
    return xrepos(!!is);
}

// Reads string
template<class CharT, class Traits, class Allocator> inline
basic_icheckstream<CharT, Traits>& operator>>(basic_icheckstream<CharT, Traits>& is, const std::basic_string<CharT, Traits, Allocator>& str)
{
    return is.read_string(const_cast<std::string&>(str), false);
}

template<class Traits> inline
basic_icheckstream<char, Traits>& operator>>(basic_icheckstream<char, Traits>& is, const signed char& ch)
{
    const char c;   // needs this, we should not deference ch in any way, may be null
    return (is >> c);
}

template<class Traits> inline
basic_icheckstream<char, Traits>& operator>>(basic_icheckstream<char, Traits>& is, const unsigned char& ch)
{
    const char c;   // needs this, we should not deference ch in any way, may be null
    return (is >> c);
}

template<class Traits> inline
basic_icheckstream<char, Traits>& operator>>(basic_icheckstream<char, Traits>& is, const signed char* s)
{
    return (is >> (const char*)s);
}

template<class Traits> inline
basic_icheckstream<char, Traits>& operator>>(basic_icheckstream<char, Traits>& is, const unsigned char* s)
{
    return (is >> (const char*)s);
}

template<class CharT, class Traits, class T> inline
basic_icheckstream<CharT, Traits>& operator>>(basic_icheckstream<CharT, Traits>&& is, const T& value)
{
    return (is >> value);
}




} // namespace datalib
