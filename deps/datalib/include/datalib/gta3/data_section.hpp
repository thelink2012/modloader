/*
 *  Copyright (C) 2014 Denilson das Mercês Amorim (aka LINK/2012)
 *  Licensed under the Boost Software License v1.0 (http://opensource.org/licenses/BSL-1.0)
 *
 */
#pragma once
#include <utility>
#include <string>
#include <array>
#include <datalib/detail/either.hpp>

namespace datalib {
namespace gta3 {

/*
 *  section_info
 *      Represents a gta3 section
 *      Construct a list of section infos by calling gta3::make_section_info(...), (e.g. gta3::make_section_info("objs", "cars", ...))
 */
// NOTE SECTIONS MUST BE SENT IN THE SAME ORDER AS DATA DECLARED IN gta3::data_section !!!!!!!!!!!!!!!!!!!!!!!
struct section_info
{
    const char* name;   // The name of this section. How it will be indentified in the sectioned file
    int         id;     // The index of this section in the sections array
    size_t      len;    // The length of the name

    section_info() : section_info(nullptr) {}
    section_info(const char* name) : name(name), id(-1), len(name? strlen(name) : 0) {}

    // Finds the a section_info object in the 'sections' array based on the specified name.
    static const section_info* by_name(const section_info* sections, const char* line)
    {
        for(auto s = sections; s->name != nullptr; ++s)
        {
            if(s->name[0])
            {
                if(!strcmp(line, s->name))
                    return s;
            }
        }
        return nullptr;
    }

    // Finds the a section_info object in the 'sections' array based on the specified name.
    // This version looks only for the first 'n' characters from the line, or if -1 based on the strlen of the section name
    static const section_info* by_name(const section_info* sections, const char* line, size_t line_strlen, int n)
    {
        for(auto s = sections; s->name != nullptr; ++s)
        {
            if(s->name[0])
            {
                size_t len = (n == -1? s->len : n);
                if(line_strlen >= len && !strncmp(line, s->name, len))
                    return s;
            }
        }
        return nullptr;
    }

    // Finds the a section_info object in the 'sections' array based on the specified name.
    static const section_info* by_name(const section_info* sections, const std::string& line)
    {
        return by_name(sections, line.data());
    }

    // Finds the a section_info object in the 'sections' array based on the specified name.
    // This version looks only for the first 'n' characters from the line, or if -1 based on the strlen of the section name
    static const section_info* by_name(const section_info* sections, const std::string& line, int n)
    {
        return by_name(sections, line.data(), line.length(), n);
    }


};

template<class... Args>
using section_array = std::array<section_info, sizeof...(Args) + 1>;

// Builds a array of section info objects, putting a null terminator and setting up the indices correctly
// NOTE SECTIONS MUST BE SENT IN THE SAME ORDER AS DATA DECLARED IN gta3::data_section !!!!!!!!!!!!!!!!!!!!!!!
template<class... Args> inline
static section_array<Args...> make_section_info(Args&&... a)
{
    auto array = section_array<Args...>( { a..., (const char*)(nullptr) } );
    for(int i = 0; i < (int(array.size()) - 1); ++i)    // last element should have a -1 id, others it's respective index
        array[i].id = i;
    return array;
}


/*
 *  data_section
 *      An set of data_slice<> objects, each one represents one of the possible sections.
 *      This works like a union, only one of the data_slice<>'s are available at a specific moment depending on the active section.
 *
 *      This object's interface is similar to data_slice<>'s interface.
 *
 */
template<typename ...Sections>
class data_section
{
    protected:
        using either_type = either<Sections...>;

        either_type data;
        const section_info* tsection = nullptr;

    public:

        static const size_t num_sections = sizeof...(Sections);

        //
        //  Constructors
        //

        data_section() = default;

        data_section(const data_section& rhs)
            : tsection(rhs.tsection), data(rhs.data)
        {}

        data_section(data_section&& rhs)
            : tsection(rhs.tsection), data(std::move(rhs.data))
        { rhs.tsection = nullptr; }

        data_section(const section_info* section)   // sets the working section after construction
        { this->force_section(section); }

        //
        //  Assignment Operators
        //

        data_section& operator=(const data_section& rhs)
        {
            this->tsection = rhs.tsection;
            this->data    = rhs.data;
            return *this;
        }

        data_section& operator=(data_section&& rhs)
        {
            this->tsection = rhs.tsection;
            this->data    = std::move(rhs.data);
            rhs.tsection   = nullptr;
            return *this;
        }

        //
        //  Comparision Operators
        //

        bool operator==(const data_section& rhs) const
        {
            if(this->has_section() && rhs.has_section())
                return (this->data == rhs.data);
            else if(!this->has_section() && !rhs.has_section())
                return true;
            return false;
        }

        bool operator<(const data_section& rhs) const
        {
            if(this->has_section() && rhs.has_section())
            {
                if(this->tsection->id == rhs.tsection->id)
                    return (this->data < rhs.data);
                return (this->tsection->id < rhs.tsection->id);
            }
            else if(!this->has_section() && !rhs.has_section())
                return false;   // both are ''equal'', no section
            return !this->has_section();    // no section goes first
        }

        //
        //
        //

        // Checks if the data present in line is compatible with the section specifier and sets the working section for this object.
        // In case the line doesn't match the section specifier, the working section gets invalidated and the method returns false
        bool as_section(const section_info* tsection, const std::string& line)
        {
            if(tsection) assert(tsection->id < num_sections);
            as_section_fn fn(*this, tsection);
            foreach_type_variadic<Sections...>()(fn);
            this->tsection = (this->check(line)? tsection : nullptr);
            return has_section();
        }

        // Forces the current section specifier to be 'tsection'
        // Be very careful when using this function
        void force_section(const section_info* tsection)
        {
            if(tsection) assert(tsection->id < num_sections);
            as_section_fn fn(*this, tsection);
            foreach_type_variadic<Sections...>()(fn);
            this->tsection = tsection;
        }


        // Gets the working section for this object
        const section_info* section() const
        {
            return this->tsection;
        }

        // Checks if there's any working section on this object
        bool has_section() const
        {
            return this->tsection != nullptr;
        }

        // Cheaply checks whether the content of 'line' can be stored in this data storer depending on the working section
        // If this return true, mostly like 'this->set(line)' will also return true
        bool check(const std::string& line) const
        {
            check_visitor visitor(line);
            return ::apply_visitor(visitor, this->data);
        }

        // Sets the content of this data storer to be what's on the line (by interpreting it)
        bool set(const std::string& line)
        {
            set_visitor visitor(line);
            return ::apply_visitor(visitor, this->data);
        }

        // Gets the content of this data storer into the line (by disassembling it)
        bool get(std::string& line)  const
        {
            get_visitor visitor(line);
            return ::apply_visitor(visitor, const_cast<either_type&>(this->data));
        }

        // Gets the nth element 'I' of type 'Type' from the working section data_slice
        // Assumes all data slices contains the same Type in the nth I element.
        template<int I, typename Type>
        Type& get() const
        {
            get_nth_visitor<I, Type> visitor;
            return (this->apply_visitor(visitor));
        }

        // Gets the nth element 'I' of type 'Type' from the working section data_slice
        // Assumes all data slices contains the same Type in the nth I element.
        template<int I, typename Type>
        Type* getp() const
        {
            get_nth_visitor_p<I, Type> visitor;
            return (this->apply_visitor(visitor));
        }

        // Gets the data_slice with the specified 'Type' (Type=some data_slice<>)
        // Throws a exception if the current bound type is not the same as 'Type'
        template<typename Type>
        Type& get_slice() const
        {
            get_slice_visitor<Type> visitor;
            return this->apply_visitor(visitor);
        }

        // CXX14 HELP-ME
        template<class Visitor>
        auto apply_visitor(Visitor& visitor) const -> decltype(::apply_visitor(std::declval<Visitor>(), std::declval<either_type>()))
        {
            return ::apply_visitor(visitor, const_cast<either_type&>(this->data));
        }

        template<class Section>
        static const section_info* section_by_slice()
        {
            static auto section = sections(*(const data_section*)(nullptr)) + tuple_elem_index<Section, std::tuple<Sections...>>::value;
            return section;
        }

    public: // not to be used directly
        template<class Archive>
        void save(Archive& ar) const
        {
            ar(data, (has_section()? tsection->id : -1));
        }
        template<class Archive>
        void load(Archive& ar)
        {
            int id;
            ar(data, id);
            this->tsection = (id != -1? sections(*this) + id : nullptr);
        }


    private:
        // CXX14 HELP-ME

        struct as_section_fn
        {
            data_section& data;
            const section_info* tsection;

            as_section_fn(data_section& data, const section_info* tsection) :
                data(data), tsection(tsection)
            {}

            template<class Integral, class TypeWr>
            bool operator()(Integral, TypeWr)
            {
                if(Integral::value == tsection->id)
                {
                    using Type = typename TypeWr::type;
                    data.data = Type();
                    return false; // stop iteration
                }
                return true;
            }
        };

        struct check_visitor : either_static_visitor<bool>
        {
            const std::string& line;
            check_visitor(const std::string& line) : line(line) {}

            template<class T>
            bool operator()(const T& value) const
            {
                return value.check(line);
            }

            bool operator()(either_blank) const
            {
                return false;
            }
        };

        struct set_visitor : either_static_visitor<bool>
        {
            const std::string& line;
            set_visitor(const std::string& line) : line(line) {}

            template<class T>
            bool operator()(T& value) const
            {
                return value.set(line);
            }

            bool operator()(either_blank) const
            {
                return false;
            }
        };

        struct get_visitor : either_static_visitor<bool>
        {
            std::string& line;
            get_visitor(std::string& line) : line(line) {}

            template<class T>
            bool operator()(T& value) const
            {
                return value.get(line);
            }

            bool operator()(either_blank) const
            {
                return false;
            }
        };

        template<int I, class Type>
        struct get_nth_visitor : either_static_visitor<Type&>
        {
            template<class T>
            Type& operator()(T& slice) const
            {
                return ::get<I>(slice);
            }

            Type& operator()(either_blank) const
            {
                throw std::runtime_error("get_nth_visitor called while object state is empty");
            }
        };

        template<int I, class Type>
        struct get_nth_visitor_p : either_static_visitor<Type*>
        {
            template<class T>
            typename std::enable_if<std::is_same<Type, std::decay_t<decltype(::<get<I>(std::declval<T>()))>>::value, Type*>::type
            /* Type* */ operator()(T& slice) const 
            {
                return &(::get<I>(slice));
            }

            template<class T>
            typename std::enable_if<!std::is_same<Type, std::decay_t<decltype(::<get<I>(std::declval<T>()))>>::value, Type*>::type
            /* Type* */ operator()(T& slice) const 
            {
                return nullptr;
            }
        };

        template<class Type>
        struct get_slice_visitor : either_static_visitor<Type&>
        {
            Type& operator()(Type& slice) const
            {
                return slice;
            }

            template<class T>
            Type& operator()(T& slice) const
            {
                throw std::runtime_error("get_slice_visitor called with wrong which");
            }

            Type& operator()(either_blank) const
            {
               throw std::runtime_error("get_slice_visitor called while object state is empty");
            }
        };


        // http://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type
        template <class T, class Tuple>
        struct tuple_elem_index;

        template <class T, class... Types>
        struct tuple_elem_index<T, std::tuple<T, Types...>> {
            static const std::size_t value = 0;
        };

        template <class T, class U, class... Types>
        struct tuple_elem_index<T, std::tuple<U, Types...>> {
            static const std::size_t value = 1 + tuple_elem_index<T, std::tuple<Types...>>::value;
        };
};


template<class T>
using data_section_visitor = either_static_visitor<T>;


// helper to find out the sections array used in a data_section
template<typename ...Sections> inline
const section_info* sections(const data_section<Sections...>& ds)
{
    static_assert(false, "sections(...) not implemented for this data_section type");
}



} // namespace gta3
} // namespace datalib

