
#include <tinympl/map.hpp>
#include <tinympl/int.hpp>
#include <tinympl/char.hpp>

namespace map_example {

using tinympl::char_;
using tinympl::int_;

typedef tinympl::map<
	std::pair<char_<'a'>,int_<0>>,
	std::pair<char_<'b'>,int_<1>>,
	std::pair<char_<'c'>,int_<2>>,
	std::pair<char_<'d'>,int_<3>>,
	std::pair<char_<'z'>,int_<25> > > char_map_t0;

static_assert(char_map_t0::size == 5 && char_map_t0::empty == false,"size && empty");
static_assert(char_map_t0::count<char_<'b'> >::type::value == 1 &&
			char_map_t0::count<char_<'f'> >::type::value == 0,"map::count");
static_assert(char_map_t0::at<char_<'b'> >::type::value == 1,"map::at");

typedef char_map_t0::erase<char_<'c'> >::type char_map_t1;
typedef char_map_t0::insert<char_<'e'>,int_<4> >::type char_map_t2;

static_assert(char_map_t1::size == 4,"erase");
static_assert(char_map_t2::size == 6,"insert");

typedef char_map_t0::insert_many<
	std::pair<char_<'e'>,int_<4> >,
	std::pair<char_<'f'>,int_<5> >,
	std::pair<char_<'g'>,int_<6> >,
	std::pair<char_<'h'>,int_<7> >,
	std::pair<char_<'a'>,int_<8> > >::type char_map_t3;
	
static_assert( char_map_t3::at<char_<'a'> >::type::value == 0, "insert_many");
static_assert( char_map_t3::at<char_<'h'> >::type::value == 7, "insert_many");
static_assert( char_map_t3::size == 9, "insert_many");

}
