/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Readme reader -- the magic is simple
 *
 */
#include "data.h"

/*
 *  ReadmeReader 
 *      This object reads a readme file, yeah, that's right, a text file.
 *      Well, not like that, it actually reads each line trying to match the formating of that line with the formating of some data traits.
 * 
 */
struct ReadmeReader
{
    const char* filename;
    char buf[2048], *line;
    size_t line_number;

    /*
     *  Helper function to get dominance flags from a integer or a functor
     */
    int GetDomFlags(int flags) { return flags; }
    template<class Functor>
    int GetDomFlags(Functor f) { return f(); /* Non-key overload */ }
    
    /*
     *  Helper function to get the readme trait (the Additional trait in other words) from a fs object 
     */
    template<class T>
    auto get_trait(T& fs, const char* file_override) -> decltype(fs.Additional(0))
    {
        return fs.Additional(file_override);
    }
    
    
    /*
     *  try_to_handle_line
     *      It's here that the magic happens my friend 
     */
    
    template<size_t N = 0, class... Tp>
    typename std::enable_if<N == sizeof...(Tp)>::type    // Use this version if reached the tuple ending
    try_to_handle_line(std::tuple<Tp...>&)
    {
    }

    template<size_t N = 0, class... Tp>
    typename std::enable_if<(N < sizeof...(Tp))>::type   // Use this version if still didn't reach the tuple end
    try_to_handle_line(std::tuple<Tp...>& t)
    {
        // Useful typedefs
        typedef typename std::tuple<Tp...>                      tuple_type;
        typedef typename std::decay<
                                    typename std::tuple_element<N, tuple_type>::type
                                   >::type                      trait_type;
        typedef typename trait_type::handler_type               handler_type;
        
        // Get trait at position N from the tuple
        trait_type& trait = std::get<N>(t);
        
        // Find out if we need to load the entire data from the original file before adding new things
        {
            int domflags = GetDomFlags(trait_type::domflags());
            
            /*  If the dominance flags say that if key do not exist in custom file it will be removed from the output,
             *  we really need to load the original data into the trait */
            if((domflags & flag_RemoveIfNotExistInAnyCustom)
            || (domflags & flag_RemoveIfNotExistInOneCustomButInDefault))
            {
                // We need to load the original data
                if(!trait.isReady) trait.LoadData();
            }
            else
            {
                // No need to load any data, the trait is ready!
                trait.isReady = true;
            }
        }
        
        // Try to add this line into the map from the trait...
        if(typename handler_type::Set()(ReadmeSection(), line, trait.map) == false)
        {
            // Failed, the formating isn't compatible, try with the next trait in the tuple
            return try_to_handle_line<N + 1, Tp...>(t);
        }
        else
        {
            // Successful! Log it.
            dataPlugin->Log("\tFound %s line at %d: %s", trait_type::what(), line_number, line);
            return;
        }
    }

    /*
     *  This object is a functor!
     *  Call me to read a readme file @filename
     */
    void operator()(const char* filename)
    {
        /*
         * XXX MAYBE (just maybe) it would be faster to read the entire file and parse from it... 
         * ...needs benchmark
         */
        
        dataPlugin->Log("Reading readme file %s", filename);
        
        // Setup my fields
        this->filename = filename;
        this->line_number = 0;
        
        // Setup the traits tuple
        auto& gta_dat    = traits.gta.Additional("data/gta.dat");
        auto& veh_ide    = traits.ide.Additional("data/vehicles.ide");
        auto  all_traits = std::tie(gta_dat, veh_ide);
        
        // Finally, open the file for reading...
        if(FILE* f = fopen(filename, "r"))
        {
            // ...read...
            for(; line = ParseConfigLine(fgets(buf, sizeof(buf), f)); ++line_number)
            {
                if(*line == 0) continue;
                try_to_handle_line(all_traits);
            }
            fclose(f);
        }
        
        // The magic is done ;)
    }
    
};


/*
 *  Reads the readme file @filename 
 */
void CThePlugin::ProcessReadmeFile(const char* filename)
{
    ReadmeReader reader;
    return reader(filename);
}
