
#include <tinympl/value_map.hpp>

namespace value_map_example {

template<char c,int i> using ci_pair = tinympl::value_map<char,int>::template pair<c,i>;

typedef tinympl::value_map<char,int,
	ci_pair<'a',0>,
	ci_pair<'b',1>,
	ci_pair<'c',2>,
	ci_pair<'d',3>,
	ci_pair<'e',4>,
	ci_pair<'z',25> > vv_map_t0;
	
static_assert( vv_map_t0::at<'c'>::value == 2,"vv_map::at");
static_assert( vv_map_t0::at<'z'>::value == 25,"vv_map::at");
static_assert( vv_map_t0::size == 6,"vv_map::size");
static_assert( vv_map_t0::count<'e'>::type::value == true &&
				vv_map_t0::count<'g'>::type::value == false,"vv_map::count");

typedef vv_map_t0::insert<'f',5>::type vv_map_t1;
static_assert( vv_map_t1::at<'f'>::value == 5,"vv_map::insert");

typedef vv_map_t0::erase<'z'>::type vv_map_t2;
static_assert( vv_map_t2::count<'z'>::value == 0, "vv_map::erase");

typedef vv_map_t0::insert_many<
	ci_pair<'f',5>,
	ci_pair<'g',6>,
	ci_pair<'h',7>,
	ci_pair<'a',8> >::type vv_map_t3;
	
static_assert( vv_map_t3::at<'a'>::type::value == 0, "insert_many");
static_assert( vv_map_t3::at<'h'>::type::value == 7, "insert_many");
static_assert( vv_map_t3::size == 9, "insert_many");

}
