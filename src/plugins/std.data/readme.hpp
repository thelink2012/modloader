/*
* Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
* Licensed under GNU GPL v3, see LICENSE at top level directory.
*
*  Readme reader -- the magic is simple
*  
*  PS1: This file should be included in data.cpp
*  PS2: Does not compile under Visual Studio (2013 and below) because of a compiler bug
*
*/
#include "data.h"

/*
*  ReadmeReader
*      This object reads a readme file, yeah, that's right, a text file.
*      Well, not like that, it actually reads each line trying to match the formating of that line with the formating of some data traits.
*
*      Please, create the additional container for each trait only if necessary
*          (when there's at least one readme with data for the specific trait)
*      that will allow optimizations.
*
*
*      PS: For the modloader overriding rule to apply, the files must be sent in the opposite order they've found/received at ProcessFile
*
*/
struct ReadmeReader
{
    const char* filename;
    char buf[2048], *line;
    size_t line_number;

    std::vector<char> readme_data;


    /*
    *  Helper function to get the readme trait (the Additional trait in other words) from a fs object
    */
    template<class T>
    auto get_trait(T& fs, const char* file_override) -> decltype(fs.GetReadmeTrait(0))
    {
        return fs.GetReadmeTrait(file_override);
    }


    /*
    *  try_to_handle_line
    *      It's here that the magic happens my friend
    */

    template<size_t N = 0, class... Tp>
    typename std::enable_if<N == sizeof...(Tp)>::type    // Use this version if reached the tuple ending
        try_to_handle_line(const std::tuple<Tp...>&)
    {
    }

    template<size_t N = 0, class... Tp>
    typename std::enable_if<(N < sizeof...(Tp))>::type   // Use this version if still didn't reach the tuple end
        try_to_handle_line(const std::tuple<Tp...>& t)
    {
            // Useful typedefs
            typedef typename std::tuple<Tp...>                          tuple_type;
            typedef typename std::tuple_element<N, tuple_type>::type    pair_type;
            typedef typename std::decay<typename pair_type::first_type>::type pair_first_type;
            typedef pair_first_type                                     fs_type;
            typedef typename fs_type::traits_type                       trait_type;
            typedef typename trait_type::handler_type                   handler_type;

            // Get trait at position N from the tuple
            typename trait_type::container_type map;
            auto& xpair = std::get<N>(t);
            fs_type& fs = xpair.first;
            const char* overrideFile = xpair.second;

            // Try to add this line into the map from the trait...
            if (typename handler_type::Set()(ReadmeSection(), line, map, overrideFile) == false)
            {
                // Failed, the formating isn't compatible, try with the next trait in the tuple
                return try_to_handle_line<N + 1, Tp...>(t);
            }
            else
            {
                auto& trait = get_trait(fs, overrideFile);

                // Find out if we need to load the entire data from the original file before adding new things
                {
                    int domflags = GetDomFlags(trait_type::domflags());

                    /*  If the dominance flags say that if key do not exist in custom file it will be removed from the output,
                    *  we really need to load the original data into the trait */
                    if ((domflags & flag_RemoveIfNotExistInAnyCustom)
                        || (domflags & flag_RemoveIfNotExistInOneCustomButInDefault))
                    {
                        // We need to load the original data
                        if (!trait.isReady) trait.LoadData();

                    }
                    else
                    {
                        // No need to load any data, the trait is ready!
                        trait.isReady = true;
                    }
                }

                // Move contents from the map on this scope to the trait map
                for (auto& x : map)
                {
                    trait.map[std::move(x.first)] = std::move(x.second);
                }

                // Successful! Log it.
                dataPlugin->Log("\tFound %s line at %d: %s", trait_type::what(), line_number, line);
                return;
            }
    }

    /*
    *  This object is a functor!
    *  Call me to read a readme file @filename
    *
    *  @tuple_pair is a tuple of std::pair, the first element on the pair must be the fs trait and the second element the filename
    *
    */
    template<class Tuple>
    void operator()(const char* filename, const Tuple& tuple_pair)
    {
        static const int max_readme_size = 10000;   // ~10KB
        typedef std::vector<char>::iterator iterator;

        // Setup my fields
        this->filename = filename;
        this->line_number = 0;

        // Read the text file only if it's not too big...
        if (GetFileSize(filename) <= max_readme_size)
        {
            dataPlugin->Log("Reading readme file %s", filename);

            // Reserve the data on the buf-vector (plus the null terminator)
            this->readme_data.reserve(max_readme_size + 1);

            // Read the entire file into our buffer
            if (ReadEntireFile(filename, this->readme_data))
            {
                // Put a null terminator at the end
                this->readme_data.emplace_back(0);

                // Iterate on each line from the file
                foreach_line(readme_data.begin(), readme_data.end(), [&tuple_pair, this](iterator sol, iterator eol)
                {
                    if (eol != this->readme_data.end())  // This condition should never be false!
                    {
                        *eol = 0;           // Make the end-of-line ('\n') be a null terminator ('\0')
                        ++line_number;      // Add to line count...

                        line = ParseConfigLine(&(*sol));
                        if (*line != 0)      // Line should not be empty...
                        {
                            // Try to detect a configuration line on the readme...
                            try_to_handle_line(tuple_pair);
                        }
                    }
                });
            }
        }
        else
        {
            //
            dataPlugin->Log("Ignoring text file \"%s\" because it's too big.", filename);
        }

        // The magic is done ;)
    }

};
