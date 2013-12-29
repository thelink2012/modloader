/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Handling Parser and Builder
 *  This code is not in the header because it's kinda huge and the header is already huge
 *  Also the handling parsing is a bit more complex than the usual.
 * 
 */

#include "data/handling.h"
#include <ordered_map.hpp>

using namespace modloader;


namespace data
{
 

/*
 *  Parses a handling file and finalizes it into map 
 */
bool TraitsHandling::Parser(const char* filename, container_type& map)
{
    return modloader::SectionParser<handler_type>(filename, map, false);
}

/*
 *  Builds a handling file based on @lines.
 *  PS: All elements in @lines must be FINAL!!! 
 */
bool TraitsHandling::Build(const char* filename, const std::vector<typename super::pair_ref_type>& lines)
{
    //         <SDataHandling_ANIM data, index>
    ordered_map<std::reference_wrapper<SDataHandling_ANIM>, int, std::equal_to<SDataHandling_ANIM>>
        animList;

    // Dummy key for anim section
    SDataHandling::key_type animKey;
    animKey.id = -1;
    animKey.section = HANDLING_ANIM;

    // Puts all anims in @animList and assign the map index to the data object
    for(auto& line : lines)
    {
        auto& final = line.second.get().final;
        
        // Insert anim from this line into animList
        final.anim.id = -127;   // to not compare the id
        auto it = animList.insert( animList.end(), std::make_pair(std::ref(final.anim), (int)(animList.size())) );
        final.anim.id = it.first->second;

        // Remove anim from this line and assign a index instead
        final.data.animgroup = final.anim.id;
        final.hasAnim = false;
    }

    // Build a new vector, adding anim lines at the end
    std::vector<SDataHandling> all_lines_data;
    std::vector<pair_ref_type> all_lines(lines);
    all_lines_data.reserve(all_lines.size() + animList.size() + 1);
    all_lines.reserve(all_lines.size() + animList.size() + 1);
    for(auto& anim : animList)
    {
        // We need a place to store our new SDataHandling data...
        all_lines_data.emplace_back();
        auto& data = all_lines_data.back();
        data.section = HANDLING_ANIM;
        data.anim    = anim.first.get();

        // Add to actual lines (animKey is dummy)
        all_lines.push_back(pair_ref_type(animKey, data));
    }
    
    
    
    // And finally call the builder
    return modloader::SectionBuilder<handler_type>(filename, all_lines, false);
}


/*
 *  Finalizes handling map, transforming everything into HANDLING_FINAL
 */
bool TraitsHandling::FinalizeData(container_type& map)
{
    // Setup first anim key for search
    SDataHandling::key_type firstAnim;
    firstAnim.section = HANDLING_ANIM;
    firstAnim.id = 0;
    
    // Find the start and end of the current map
    container_type::iterator basic_begin = map.begin();
    container_type::iterator basic_end   = map.end();
    
    // Deletion position iterators
    container_type::iterator del_begin  = basic_begin;
    container_type::iterator del_end    = basic_end;
    
    // Point iterators
    container_type::iterator data_begin  = basic_end;
    container_type::iterator data_end    = basic_end;
    container_type::iterator boat_begin  = basic_end;
    container_type::iterator boat_end    = basic_end;
    container_type::iterator bike_begin  = basic_end;
    container_type::iterator bike_end    = basic_end;
    container_type::iterator plane_begin = basic_end;
    container_type::iterator plane_end   = basic_end;
    container_type::iterator anim_begin  = basic_end;
    container_type::iterator anim_end    = basic_end;
    
    // Point iterator assigner:
    auto assign_iterator = [&](uint8_t section, bool end, const container_type::iterator& src)
    {
        switch(section)
        {
            case HANDLING_DATA:
                if(end) data_end = src; 
                else    data_begin = src;
                break;
                
            case HANDLING_BOAT:
                if(end) boat_end = src; 
                else    boat_begin = src;
                break;
                
            case HANDLING_BIKE:
                if(end) bike_end = src; 
                else    bike_begin = src;
                break;
                
            case HANDLING_PLANE:
                if(end) plane_end = src; 
                else    plane_begin = src;
                break;
                
            case HANDLING_ANIM:
                if(end) anim_end = src; 
                else    anim_begin = src;
                break;
        }
    };
    
    // Find key assigned with hash in the range begin-end
    auto find_hash = [](container_type::iterator begin, container_type::iterator end, size_t hash)
    {
        return std::find_if(begin, end, [&hash](const container_type::value_type& a)
        {
            return(a.first.name.hash == hash);
        });
    };
    
    // Setups additional data for the final object
    auto setup_additional_data = [&](SDataHandling_FINAL& final)
    {
        container_type::iterator it;
        size_t hash = final.data.name.hash;
        
        // Try to find the vehicle in the section for boats...
        it = find_hash(boat_begin, boat_end, hash);
        if(it != boat_end)
        {
            final.additional = HANDLING_BOAT;
            final.boat = it->second.boat;
            return;
        }
        
        // Try to find the vehicle in the section for bikes...
        it = find_hash(bike_begin, bike_end, hash);
        if(it != bike_end)
        {
            final.additional = HANDLING_BIKE;
            final.bike = it->second.bike;
            return;
        }
        
        // Try to find the vehicle in the section for planes...
        it = find_hash(plane_begin, plane_end, hash);
        if(it != plane_end)
        {
            final.additional = HANDLING_PLANE;
            final.plane = it->second.plane;
            return;
        }
        
        // The vehicle isn't in any additional section
        final.additional = HANDLING_NONE;
        return;
    };
    
    
    // Travel in the map building iterator positions
    uint8_t curr_section = HANDLING_NONE;
    for(auto it = basic_begin; it != basic_end; ++it)
    {
        if(it->first.section != curr_section)
        {
            // Resetup deleletin begging position if there's already any handling final structure on the map
            // Setup to the first element that's non-final
            if(curr_section == HANDLING_FINAL)
                del_begin = it;

            // Assign end and begin for iterators
            assign_iterator(curr_section, true, it);
            curr_section = it->first.section;
            assign_iterator(curr_section, false, it);
        }
    }
    
    // If there's no data to finalize (everything is finalized or something), ignore the request
    if(data_begin == data_end)
        return false;

    // Vector with anims, the vector index is the same as the anim id
    std::vector<SDataHandling_ANIM*> anim;
    anim.reserve(32);
    
    // Build the anim vector
    for(auto it = anim_begin; it != anim_end; ++it)
    {
        int id = it->first.id;
        if(id != anim.size()) anim.resize(id);  // Put some gap in the vector if skipping any id
        anim.push_back(&it->second.anim);
    }
    
    // Build the finalized data
    {
        SDataHandling::key_type final_key;
        final_key.section = HANDLING_FINAL;

        for(auto it = data_begin; it != data_end; ++it)
        {
            // Put a new final key into the map
            final_key.name.hash = it->first.name.hash;
            auto final_it = map.emplace_hint(map.begin(), final_key, SDataHandling());
            // Setup the new element in the map
            final_it->second.section = HANDLING_FINAL;
            SDataHandling_FINAL& final = final_it->second.final;
            
            // it data
            SDataHandling_DATA& data = it->second.data;
            
            // Setup final structure
            final.data = std::move(data);
            final.anim = *anim.at(data.animgroup);
            setup_additional_data(final); 
            final.hasAnim = true;
            final.data.animgroup = -1;
        }
    }
   
    // Delete all non-final data
    map.erase(del_begin, del_end);
    return true;
}


/*
 *  Some people don't know how to write a proper handling.cfg line, since the game don't use sscanf (but strtok/atoi/atof)
 *  to parse handling.cfg those errors are not detected by the game itself.
 *  To not just make those bad mods incompatible, let's fix the problem ourself on the fly.
 * 
 *  Common problems are:
 *      Using two dots on a float (seriosly), like: 1778.58.02
 *      Using floats where integers are requiered (such as in nPercentSubmerged)
 *      Puting a char (e.g. to represent seconds) after a number, R* itself did this...
 * 
 */
char* SDataHandling::TryToFixLine(const char* line, char* output)
{
    using namespace modloader::parser;
    
    char* result = output;
    const char* format = "";

    // Get formating
    switch(this->section)
    {
        case HANDLING_DATA:     format = data.format();  break;
        case HANDLING_BOAT:     format = boat.format();  break;
        case HANDLING_BIKE:     format = bike.format();  break;
        case HANDLING_PLANE:    format = plane.format(); break;
    }
    
    while(*format)
    {
        // Skip spaces
        while(isspace(*format)) ++format;
        while(isspace(*line)) ++line;

        // Current character from format is an '%', get to the next one
        char f = *++format; ++format; 
        // Now format is on a space character

        // If line already ended, we have a real problem
        // You know what I'll do? Just output a "0" since it's accepted as a int, float or string
        if(*line == 0)
        {
            *output++ = '0';
        }
        else
        {
            bool bFoundDot = false;
            
            // Parse the line until next format
            while(*line && !isspace(*line))
            {
                if(f != 's' && f != 'c' && f != 'x')
                {
                    if(*line == '.')
                    {
                        // If found dot but it has been already found or the format asks for a integer,
                        // get out of the parsing
                        if(f == 'd' || (f == 'f' && bFoundDot)) break;
                        bFoundDot = true;
                    }
                    else
                    {
                        // If not a string (neither char), it must be a numeric character, right?
                        if(!isdigit(*line) && *line != '-' && *line != '+')
                        {
                            if(f != 'f') break;
                            if(*line != 'e' && *line != 'E') break; // Float exponent
                        }
                    }
                }

                // Transfer char from line to output
                *output++ = *line++;
            }
        }

        // Put separation at output
        *output++ = ' ';

        // Skip non-spaces
        while(*line && !isspace(*line)) ++line;
    }
    
    // end string and return it
    *output = '\0';
    return result;
}    



}   // namespace
