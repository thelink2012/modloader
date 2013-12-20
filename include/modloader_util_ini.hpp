/* 
 * San Andreas Mod Loader Utilities Headers
 * Created by LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This file provides helpful functions for plugins creators.
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
#ifndef MODLOADER_UTIL_INI_HPP
#define	MODLOADER_UTIL_INI_HPP

/*
 *  STL-like INI Container
 */

#include <string>
#include <map>
#include <cstdio>

#include "modloader_util_container.hpp"


namespace modloader
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
            
            /* Too lazy to continue this container, TODO */
            
#if 1
            bool load_file(const char_type* filename)
            {
                /* Using C stream in a STL-like container, funny?
                 * No. For me, C++ streams are bad designed, non friendly interface, I don't like it
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
 
                    while(fgets(buf, sizeof(buf), f))
                    {
                        // What a thing, reading into a char buffer and then putting in the string...
                        line = buf;
                        
                        // Find comment and remove anything after it from the line
                        if((pos = line.find_first_of(';')) != line.npos)
                            line.erase(pos);
                        
                        // Trim the string, and if it gets empty, skip this line
                        if(TrimString(line).empty())
                            continue;
                        
                        // Find section name
                        if(line[0] == '[')
                        {
                            pos = line.find_first_of(']');
                            if(pos != line.npos)
                            {
                                TrimString(key.assign(line, 1, pos-1));
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
                                TrimString(key.assign(line, 0, pos), false, true);                  // trim the right
                                TrimString(value.assign(line, pos + 1, line.npos), true, false);    // trim the left
                            }

                            // Put the key/value into the current keys object, or into the section "" if no section has been found
                            (keys? *keys : data.at(null_string)).emplace(std::move(key), std::move(value));
                        }
                    }
                    
                    fclose(f);
                    return true;
                }
                return false;
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
    
#endif	/* MODLOADER_UTIL_INI_HPP */

