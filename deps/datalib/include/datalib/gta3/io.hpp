/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <set>
#include <vector>
#include <fstream>
#include <string>
#include <functional>
#include <datalib/gta3/data_section.hpp>

namespace datalib {
namespace gta3 {


//
//  this header file provides I/O and Merging for gta3 data files
//



/*
 *  trim_config_line
 *      Trims a config line just like gta3 does internally
 *      Essentially removes all comments (';', '#"), replaces ',' and space characters with ' ' and trims left and right.
 */
inline std::string& trim_config_line(std::string& line, bool remove_separators = true)
{
    bool trim_front = true;
    std::size_t trim_back = line.npos;

    for(std::size_t pos = 0; pos < line.length(); ++pos)
    {
        char c = line[pos];
        if(c <= ' ' || c == ',')
        {
            if(c != ',' || remove_separators)
            {
                if(trim_back == line.npos) trim_back = pos;
                line[pos] = ' ';
            }
        }
        else if(c == '#' || c == ';')
        {
            if(trim_back == line.npos) trim_back = pos;
            line.erase(pos);
            break;
        }
        else
        {
            trim_back = line.npos;
            if(!trim_front)
            {
                // first non-whitespace char
                trim_front = false;
                line.erase(0, pos);
                pos = 0;
            }
        }
    }

    if(trim_back != line.npos)
        line.erase(trim_back);

    return line;
}

/*
 *  getline
 *      Gets the current line in the character iterator 'in' (.first is begin and .second is end)
 *      Modifies begin (.first) the pair of iterators to point to the next line.
 *      Similar to std::getline but using forward iterators.
 */
template<class ForwardIterator>
inline bool getline(std::pair<ForwardIterator, ForwardIterator>& in, std::string& outstr)
{
    ForwardIterator &begin = in.first, &end = in.second;
    if(begin != end)
    {
        ForwardIterator first = begin, last = end;

        for(; begin != end; ++begin)
        {
            if(*begin == '\n' || *begin == 0)
            {
                last = begin++;
                break;
            }
        }

        outstr.assign(first, last);
        return true;
    }
    return false;
}




/*
 *  parse_from_stream
 *      Functor which parses the content of an stream and inserts it into a store.
 *      Also accepts a pair of iterators as parameter instead of a stream.
 */
struct parse_from_stream
{
    public:

        template<class StoreType, class CharT, class CharTraits>
        bool operator()(StoreType& store, std::basic_istream<CharT, CharTraits>& stream) const
        {
            return this->read(store, stream);
        }

        template<class StoreType, class ForwardIterator>
        bool operator()(StoreType& store, std::pair<ForwardIterator, ForwardIterator>& bufpair) const
        {
            return this->read(store, bufpair);
        }

    private:

        static const std::size_t line_reserve = 512;

        template<class StoreType, class StreamType>
        bool read(StoreType& store, StreamType& stream) const
        {
            if(doread(store, stream))
                return store.posread();
            return false;
        }

        template<class StoreType, class StreamType>
        typename std::enable_if<StoreType::has_sections, bool>::type
        /* bool */ doread(StoreType& store, StreamType& stream) const
        {
            return read_withsec(store, stream);
        }

        template<class StoreType, class StreamType>
        typename std::enable_if<!StoreType::has_sections, bool>::type
        /* bool */ doread(StoreType& store, StreamType& stream) const
        {
            return read_woutsec(store, stream);
        }


        template<class StoreType, class StreamType>
        bool read_withsec(StoreType& store, StreamType& stream) const
        {
            std::string line; line.reserve(line_reserve);
            const section_info* sections = store.sections();
            bool per_line_section = store.per_line_section;

            const section_info* section = nullptr;

            while(getline(stream, line))
            {
                if(trim_config_line(line).size())
                {
                    // take care of sectioning
                    if(section == nullptr || per_line_section)
                    {
                        section = store.section_by_line(sections, line);
                        if(!per_line_section) continue;
                    }
                    else if(!strcmp(line.data(), "end"))
                    {
                        section = nullptr;
                        continue;
                    }

                    // read the line as the specified section
                    if(section != nullptr)
                    {
                        if(!store.insert(section, line))
                        {
                            // tolerant to this kind of failure
                        }
                    }
                }
            }
            return true;
        }

        template<class StoreType, class StreamType>
        bool read_woutsec(StoreType& store, StreamType& stream) const
        {
            std::string line; line.reserve(line_reserve);

            while(getline(stream, line))
            {
                if(trim_config_line(line).size())
                {
                    if((store.insert(nullptr, line)) == false)
                    {
                        // tolerant to this kind of failure
                    }
                }
            }

            return true;
        }

};

/*
 *  parse_from_file
 *      Functor which parses the content of an file name and inserts it into a store.
 */
struct parse_from_file
{
    // If the file has the size greater than this, it'll parse using a stream otherwise reading it completly into memory and then parsing there.
    static const std::streamoff max_size_for_memory = 2097152;  // 2MiB

    template<class StoreType>
    bool operator()(StoreType& store, const char* filename) const
    {
        std::ifstream stream(filename, std::ios::binary);   // open as binary even if we are working with text
                                                            // first because well need to determine the file size
                                                            // and why to reopen it if getline works here and we gonna trim line endings?
        if(stream)
        {
            std::streamoff filesize;
            if(stream.seekg(0, std::ios::end) && ((filesize = stream.tellg()) != std::streamoff(-1)))
            {
                if(filesize <= max_size_for_memory)
                {
                    auto ufilesize = (size_t)filesize;
                    std::unique_ptr<char[]> buffer(new char[ufilesize]);
                    if(stream.seekg(0, std::ios::beg) && stream.read(&buffer[0], filesize))
                        return parse_from_stream()(store, std::make_pair(&buffer[0], &buffer[ufilesize]));
                }
            }

            // reading into memory buffer didn't happen, then read line by line
            if(stream.seekg(0, std::ios::beg))
                return parse_from_stream()(store, stream);
        }
        return false;
    }
};





/*
 *  store_merger
 *      Functor which merges many data_store's and outputs the result into a file
 */
struct store_merger
{
    public:

        //
        // NOTE: It's dangerous for the game to leave any blank line in the output file
        //       For example plants.dat reads everything without care using strtok
        //


        // Merges the data stores in the iterator [st_begin, st_end] and outputs the result into the file 'outfilename'
        // Notice 'domflags' is a functor which returns the dominance flag based on the key sent to it
        template<class StoreType, class ForwardIterator, class DomFlags>
        bool operator()(const char* outfilename, ForwardIterator st_begin, ForwardIterator st_end, DomFlags domflags)
        {
            std::ofstream stream(outfilename);
            if(stream)
            {
                std::for_each(st_begin, st_end, [](StoreType& store) { store.premerge(); });
                auto result = do_merge<StoreType>(stream, st_begin, st_end, domflags);
                std::for_each(st_begin, st_end, [](StoreType& store) { store.posmerge(); });
                return result;
            }
            return false;
        }

    private:

        static const std::size_t line_reserve = 512;

        template<class Key>
        using keylist_sorted_type = std::set<std::reference_wrapper<const Key>, std::less<Key>> ;
        template<class Key>
        using keylist_ordered_type = std::vector<std::reference_wrapper<const Key>> ;

        // Write helper to select between write_withsec/write_woutsec using the integral_constant boolean as the first parameter
        template<class StoreType, class ForwardIterator, class StreamType>
        bool write(std::true_type has_section, ForwardIterator begin, ForwardIterator end, StreamType& stream) const
        {
            return this->write_withsec<StoreType>(begin, end, stream);
        }
        template<class StoreType, class ForwardIterator, class StreamType>
        bool write(std::false_type has_section, ForwardIterator begin, ForwardIterator end, StreamType& stream) const
        {
            return this->write_woutsec<StoreType>(begin, end, stream);
        }

        // Writes for stores with section
        template<class StoreType, class ForwardIterator, class StreamType>
        bool write_withsec(ForwardIterator begin, ForwardIterator end, StreamType& stream) const
        {
            std::string line; line.reserve(line_reserve);
            const section_info* sections = StoreType::sections();
            bool per_line_section = StoreType::per_line_section;

            const section_info* gsection = nullptr;

            // Updates the current working section
            auto change_section = [&](const section_info* newsec)
            {
                if(!per_line_section)
                {
                    if(gsection != newsec)
                    {
                        if(gsection) stream << "end\n";
                        if(newsec) stream << newsec->name << '\n';
                        gsection = newsec;
                    }
                }
            };

            for(auto it = begin; it != end; ++it)
            {
                if(auto* section = StoreType::section_by_kv(it->first, it->second))
                {
                    change_section(section);
                    if(StoreType::getline(it->first, it->second, line))
                        stream << line << '\n';
                }
            }

            change_section(nullptr);    // write a final "end"
            return true;
        }

        // Writer for stores without sections
        template<class StoreType, class ForwardIterator, class StreamType>
        bool write_woutsec(ForwardIterator begin, ForwardIterator end, StreamType& stream) const
        {
            std::string line; line.reserve(line_reserve);
            for(auto it = begin; it != end; ++it)
            {
                if(StoreType::getline(it->first, it->second, line))
                    stream << line << '\n';
            }
            return true;
        }




        // Pushes the key to the container if there's none equal to it yet
        template<class Key>
        bool push_unique(keylist_sorted_type<Key>& list, const Key& key)
        {
            // fine, this is a std::set, defaults to unique anyway
            return list.emplace(std::cref(key)).second;
        }

        // Pushes the key to the container if there's none equal to it yet
        template<class Key>
        bool push_unique(keylist_ordered_type<Key>& list, const Key& key)
        {
            if(std::find_if(list.begin(), list.end(), [&key](const Key& a)
            {
                return (a == key);
            }) == list.end())
            {
                list.emplace_back(std::cref(key));
                return true;
            }
            return false;
        }

        template<class ForwardIterator>
        using merge_result = std::vector<std::pair<
                                std::reference_wrapper<std::add_const_t<typename std::iterator_traits<ForwardIterator>::value_type::key_type>>,
                                std::reference_wrapper<typename std::iterator_traits<ForwardIterator>::value_type::mapped_type>
                            >>;

        // Merges the data_store range [st_begin, st_end] and outputs a list of pairs with the results
        template<class ForwardIterator, class DomFlags>
        auto merge(ForwardIterator st_begin, ForwardIterator st_end, DomFlags domflags) ->
                merge_result<ForwardIterator>
        {
            // Aliases
            using store_type = std::iterator_traits<ForwardIterator>::value_type;
            using key_type = typename store_type::key_type;
            using mapped_type = typename store_type::mapped_type;

            //using relist_type = std::vector < std::pair <
            //    std::reference_wrapper<const key_type>, std::reference_wrapper < mapped_type >
            //    >> ;
            using relist_type = merge_result<ForwardIterator>;

            using keylist_type =
                std::conditional < store_type::is_sorted,
                std::set<std::reference_wrapper<const key_type>, std::less<key_type>>,
                std::vector < std::reference_wrapper<const key_type> >
                > ::type;

            // Actual code is here lol

            keylist_type keys;
            relist_type  output;

            for(auto st = st_begin; st != st_end; ++st)
            {
                if(st->ready())
                {
                    for(auto& kv : st->container())
                        push_unique(keys, kv.first);
                }
            }

            for(auto& key : keys)
            {
                if(auto* vdom = find_dominant_data(st_begin, st_end, key.get(), domflags(key.get())))
                    output.emplace_back(key, *vdom);
            }

            return output;
        }

        // CXX14 HELP ME

        template<class StoreType>
        struct fn_dowrite
        {
            fn_dowrite(store_merger& merger, std::ofstream& stream) :
                merger(merger), stream(stream)
            {}

            template<class MergedList>
            bool operator()(const MergedList& mlist) const
            {
                return merger.write<StoreType>(
                    std::integral_constant<bool, StoreType::has_sections>(),
                    mlist.begin(), mlist.end(), stream);
            }

            private:
            store_merger& merger;
            std::ofstream& stream;
        };

        template<class StoreType, class ForwardIterator, class DomFlags>
        bool do_merge(std::false_type, std::ofstream& stream, ForwardIterator st_begin, ForwardIterator st_end, DomFlags domflags)
        {
            return StoreType::prewrite(merge(st_begin, st_end, domflags), fn_dowrite<StoreType>(*this, stream));
        }

        template<class StoreType, class ForwardIterator, class DomFlags>
        bool do_merge(std::true_type, std::ofstream& stream, ForwardIterator st_begin, ForwardIterator st_end, DomFlags domflags)
        {
            auto xc = StoreType::traits_type::process_stlist<StoreType>(st_begin, st_end);
            return StoreType::prewrite(merge(xc.begin(), xc.end(), domflags), fn_dowrite<StoreType>(*this, stream));
        }

        template<class StoreType, class ForwardIterator, class DomFlags>
        bool do_merge(std::ofstream& stream, ForwardIterator st_begin, ForwardIterator st_end, DomFlags domflags)
        {
            return do_merge<StoreType>(std::integral_constant<bool, StoreType::traits_type::do_stlist>(),
                                              stream, st_begin, st_end, domflags);
        }

};


/*
 *  merge_to_file
 *      Merges the stores in the range [st_begin, st_end] into the file 'outfilename'
 *      Notice 'domflags' is a functor which returns the dominance flag based on the key sent to it
 */
template<class StoreType, class ForwardIterator, class DomFlags> inline
bool merge_to_file(const char* outfilename, ForwardIterator st_begin, ForwardIterator st_end, DomFlags domflags)
{
    store_merger merger;
    return merger.operator()<StoreType>(outfilename, st_begin, st_end, domflags);
}



} // namespace gta3
} // namespace datalib

