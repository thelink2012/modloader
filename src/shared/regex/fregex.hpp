/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include "regex.hpp"

/*
 *  Compiles a regex based on a sscanf format (which also accepts regexing)
 *      The format is as follow:
 *          %s  -> Matches a string (as in '\w+')
 *          %f  -> Matches a floating point
 *          %d  -> Matches an integer (as in '\d+')
 *          %x  -> Matches a hexadecimal integer
 *          %c  -> Matches a single character
 *          %{  -> Matches a regex in between the brackets
 *          %${ -> Matches a regex in between the brackets as a capture group
 *          
 *      Using regexes outside %{ and %$ is supported, but only needed in some cases, in any other case use the formats.
 *
 */
struct fregex_compiler
{
    std::string str;

    void begin_compilation()
    {
    }

    void add_float()
    {
        str += R"___([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)___";
    }

    void add_integer()
    {
        str += R"___([+-]?\d+)___";
    }

    void add_hexa()
    {
        str += R"___([+-]?(?:0[xX])?[\dA-Fa-f]+)___";
    }

    void add_string()
    {
        str += R"___(\S+)___";
    }

    void add_char()
    {
        str += R"___(.)___";
    }

    void add_space()
    {
        str += R"___(\s+)___";
    }

    void finish_compilation()
    {
        str += R"___(\s*)___";
    }

    template<class ForwardIt>
    ForwardIt compile_regexy(ForwardIt begin, ForwardIt end, bool capture)
    {
        auto it = begin;
        if(*it == '{')
        {
            str.append(capture? "(" : "(?:");
            size_t count = 0;
            for(auto prev = *it++; it != end; prev = *it++)
            {
                if(prev != '\\')
                {
                    if(*it == '}')
                    {
                        if(count == 0) break;
                        else --count;
                    }
                    else if(*it == '{') ++count;
                }
                str.push_back(*it);
            }
            str.append(")");
        }
        return it;
    }

    template<class ForwardIt>
    fregex_compiler& compile(ForwardIt begin, ForwardIt end)
    {
        std::string output;
        output.reserve(std::distance(begin, end) * 3);
        
        begin_compilation();
        auto prev = '\0';
        bool was_space = false;
        for(auto it = begin; it != end; prev = *it++)
        {
            if(*it == ' ')
            {
                if(!was_space) add_space();
                was_space = true;
                continue;
            }
            else was_space = false;

            if(*it == '%')
            {
                if(std::next(it) != end)
                {
                    switch(*++it)
                    {
                        case 'x': case 'X':
                            add_hexa(); continue;
                        case 'd':
                            add_integer(); continue;
                        case 'f':
                            add_float(); continue;
                        case 's':
                            add_string(); continue;
                        case 'c':
                            add_char(); continue;
                        case '%':
                            str.push_back('%'); continue;
                        case '{':
                            it = compile_regexy(it, end, false); continue;
                        case '$':
                            if(*std::next(it) == '{')
                                it = compile_regexy(std::next(it), end, true);
                            continue;
                        default:
                            throw std::runtime_error("fregex: invalid formating");
                    }
                }
            }

            str.push_back(*it);
        }
        finish_compilation();
        return *this;
    }
    
    fregex_compiler& compile(const std::string& rgx)
    {
        compile(rgx.begin(), rgx.end());
        return *this;
    }

    const std::string& result() const
    {
        return str;
    }
};

inline sregex make_fregex(const std::string& begin, sregex::flag_type flags = sregex::ECMAScript|sregex::optimize)
{
    return make_regex(fregex_compiler().compile(begin).result(), flags);
}

