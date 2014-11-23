/* 
 *  Copyright (c) 2013-2014 Denilson das Mercês Amorkm <dma_2012@hotmail.com>
 *  
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 * 
 */
#ifndef LINB_INI_PARSER_HPP
#define LINB_INI_PARSER_HPP

/*
 *  STL-like INI Container
 */

#include <string>       // for std::string
#include <map>          // for std::map
#include <cstdio>       // for std::FILE
#include <algorithm>    // for std::find_if
#include <functional>   // for std::function

namespace linb
{
    template<
        class CharT             = char,     /* Not compatible with other type here, since we're using C streams */
        class StringType        = std::basic_string<CharT>,
        class KeyContainer      = std::map<StringType, StringType>,
        class SectionContainer  = std::map<StringType, KeyContainer>
    > class basic_ini
    {
        public:
            typedef CharT               char_type;
            typedef StringType          string_type;
            typedef KeyContainer        key_container;
            typedef SectionContainer    section_container;
            
            // Typedef container values types
            typedef typename section_container::value_type              value_type;
            typedef typename section_container::key_type                key_type;
            typedef typename section_container::mapped_type             mapped_type;
            
            // Typedef common types
            typedef typename section_container::size_type               size_type;
            typedef typename section_container::difference_type         difference_type;
            
            // Typedef iterators
            typedef typename section_container::iterator                iterator;
            typedef typename section_container::const_iterator          const_iterator;
            typedef typename section_container::reverse_iterator        reverse_iterator;
            typedef typename section_container::const_reverse_iterator  const_reverse_iterator;
            
            // typedef References and pointers
            typedef typename section_container::reference               reference;
            typedef typename section_container::const_reference         const_reference;
            typedef typename section_container::pointer                 pointer;
            typedef typename section_container::const_pointer           const_pointer;
            
        private:
            section_container data;
            
        public:
            
            basic_ini()
            { }

            basic_ini(const char_type* filename)
            { this->read_file(filename); }
            
            /* Iterator methods */
            iterator begin()
            { return data.begin(); }
            const_iterator begin() const
            { return data.begin(); }
            iterator end()
            { return data.end(); }
            const_iterator end() const
            { return data.end(); }
            const_iterator cbegin() const
            { return data.cbegin(); }
            const_iterator cend() const
            { return data.cend(); }
            
            /* Reverse iterator methods */
            reverse_iterator rbegin()
            { return data.rbegin(); }
            const_reverse_iterator rbegin() const
            { return data.rbegin(); }
            reverse_iterator rend()
            { return data.rend(); }
            const_reverse_iterator rend() const
            { return data.rend(); }
            const_reverse_iterator crbegin() const
            { return data.crbegin(); }
            const_reverse_iterator crend() const
            { return data.crend(); }
            
            /* Acessing index methods */
            mapped_type& operator[](const key_type& key)
            { return data[key]; }
            mapped_type& operator[](key_type&& key)
            { return data[std::forward<key_type>(key)]; }
            mapped_type& at( const key_type& key)
            { return data.at(key); }
            const mapped_type& at(const key_type& key) const
            { return data.at(key); }
            
            /* Capacity information */
            bool empty() const
            { return data.empty(); }
            size_type size() const
            { return data.size(); }
            size_type max_size() const
            { return data.max_size(); }
            
            /* Modifiers */
            void clear()
            { return data.clear(); }
            
            /* Too lazy to continue this container... If you need more methods, just add it */
            
#if 1
            bool read_file(const char_type* filename)
            {
                /* Using C stream in a STL-like container, funny?
                 */
                if(FILE* f = fopen(filename, "r"))
                {
                    key_container* keys = nullptr;
                    char_type buf[2048];
                    string_type line;
                    string_type key;
                    string_type value;
                    string_type null_string;
                    size_type pos;
                    
                    // Trims an string
                    auto trim = [](string_type& s, bool trimLeft, bool trimRight) -> string_type&
                    {
                        if(s.size())
                        {
                            // Ignore UTF-8 BOM
                            while(s.size() >= 3 && s[0] == (char)(0xEF) && s[1] == (char)(0xBB) && s[2] == (char)(0xBF))
                                s.erase(s.begin(), s.begin() + 3);

                            if(trimLeft)
                                s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::function<int(int)>(::isspace))));
                            if(trimRight)
                                s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::function<int(int)>(::isspace))).base(), s.end());
                        }
                        return s;
                    };
                    
                    // Start parsing
                    while(fgets(buf, sizeof(buf), f))
                    {
                        // What a thing, reading into a char buffer and then putting in the string...
                        line = buf;
                        
                        // Find comment and remove anything after it from the line
                        if((pos = line.find_first_of(';')) != line.npos)
                            line.erase(pos);
                        
                        // Trim the string, and if it gets empty, skip this line
                        if(trim(line, true, true).empty())
                            continue;
                        
                        // Find section name
                        if(line.front() == '[' && line.back() == ']')
                        {
                            pos = line.length() - 1; //line.find_first_of(']');
                            if(pos != line.npos)
                            {
                                trim(key.assign(line, 1, pos-1), true, true);
                                keys = &data[std::move(key)];  // Create section
                            }
                            else
                                keys = nullptr;
                        }
                        else
                        {
                            // Find key and value positions
                            pos = line.find_first_of('=');
                            if(pos == line.npos)
                            {
                                // There's only the key
                                key = line;         // No need for trim, line is already trimmed
                                value.clear();
                            }
                            else
                            {
                                // There's the key and the value
                                trim(key.assign(line, 0, pos), false, true);                  // trim the right
                                trim(value.assign(line, pos + 1, line.npos), true, false);    // trim the left
                            }

                            // Put the key/value into the current keys object, or into the section "" if no section has been found
                            #if __cplusplus >= 201103L || _MSC_VER >= 1800
                            (keys ? *keys : data[null_string]).emplace(std::move(key), std::move(value));
                            #else
                            (keys ? *keys : data[null_string])[key] = value;
                            key.clear(); value.clear();
                            #endif
                        }
                    }
                    
                    fclose(f);
                    return true;
                }
                return false;
            }

            /*
             *  Dumps the content of this container into an ini file
             */
            bool write_file(const char_type* filename)
            {
                if(FILE* f = fopen(filename, "w"))
                {
                    for(auto& sec : this->data)
                    {
                        fprintf(f, "\n[%s]\n", sec.first.c_str());
                        for(auto& kv : sec.second)
                            fprintf(f, "%s = %s\n", kv.first.c_str(), kv.second.c_str());
                    }
                    fclose(f);
                    return true;
                }
                return false;
            }


            /*
            */
            bool load_file(const char_type* filename)
            {
                return read_file(filename);
            }

            bool load_file(const StringType& filename)
            {
                return load_file(filename.c_str());
            }

            bool write_file(const StringType& filename)
            {
                return write_file(filename.c_str());
            }
#endif       
            

        
    };
    
    
    /* Use default basic_ini
     * 
     *  Limitations:
     *      * Not unicode aware
     *      * Case sensitive
     *      * Sections must have unique keys
     */
    typedef basic_ini<>     ini;
}
    
#endif

