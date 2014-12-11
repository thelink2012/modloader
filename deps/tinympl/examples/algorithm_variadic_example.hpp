
#include <tuple>
#include <tinympl/algorithm_variadic.hpp>
#include <tinympl/functional.hpp>

namespace algorithm_variadic_example {

//erase
static_assert(
	std::is_same<
		tinympl::variadic::erase< 2,4,std::tuple,int,char,long,double,float,short>::type,
		std::tuple<int,char,float,short>
	>::value,"erase");

//insert
static_assert(
	std::is_same<
		tinympl::variadic::insert< 3,void*,std::tuple,int,char,long,double,float,short>::type,
		std::tuple<int,char,long,void*,double,float,short>
	>::value,"insert");

//count_if
static_assert(
	tinympl::variadic::count_if< std::is_floating_point,
		float,double,char,int,long>::type::value == 2,"count_if");

// count
static_assert(
	tinympl::variadic::count< int,
		float,int,long,int,char>::type::value == 2,"count");

//find_if
static_assert(
	tinympl::variadic::find_if< std::is_const,
		char,long,float,const int,double>::type::value == 3,"find_if");

//find
static_assert(
	tinympl::variadic::find< double,
		char,int,double,unsigned int>::type::value == 2,"find");

//copy_if
static_assert(
	std::is_same<
		tinympl::variadic::copy_if< std::is_floating_point, std::tuple,
			int,double,float,long double,char>::type,
		std::tuple<double,float,long double> >::value,"copy_if");

//copy_n
static_assert(
	std::is_same<
		tinympl::variadic::copy_n< 3, std::tuple,
			int,long,double,char>::type,
		std::tuple<int,long,double> >::value,"copy_n");

//fill_n
static_assert(
	std::is_same<
		tinympl::variadic::fill_n< 2, int, std::tuple >::type,
		std::tuple<int,int> >::value,"fill_n");

//generate_n
static_assert(
	std::is_same<
		tinympl::variadic::generate_n<4,
			tinympl::bind< tinympl::multiplies, tinympl::arg1, tinympl::int_<2> >::template eval, std::tuple>::type,
		std::tuple< tinympl::int_<0>,tinympl::int_<2>,tinympl::int_<4>,tinympl::int_<6> >
	>::value,"generate_n");

//transform
static_assert(
	std::is_same<
		tinympl::variadic::transform< std::add_const, std::tuple,
			int,char>::type,
		std::tuple<const int,const char> >::value,"transform");

//replace_if
static_assert(
	std::is_same<
		tinympl::variadic::replace_if< std::is_array, void*,std::tuple,
			int,int[],char>::type,
		std::tuple<int,void*,char> >::value,"replace_if");

//replace
static_assert(
	std::is_same<
		tinympl::variadic::replace< char, unsigned char,std::tuple,
			int,int[],char>::type,
		std::tuple<int,int[],unsigned char> >::value,"replace");

//remove_if
static_assert(
	std::is_same<
		tinympl::variadic::remove_if< std::is_integral, std::tuple,
			int,double,float>::type,
		std::tuple<double,float> >::value,"remove_if");

//remove
static_assert(
	std::is_same<
		tinympl::variadic::remove< char, std::tuple,
			long,char,int>::type,
		std::tuple<long,int> >::value,"remove");

using tinympl::int_;

//min_element
static_assert(
	tinympl::variadic::min_element<tinympl::less, int_<4>,int_<3>,int_<8>,int_<10> >::type::value == 1,"min_element");

//max_element
static_assert(
	tinympl::variadic::max_element<tinympl::less, int_<4>,int_<3>,int_<8>,int_<10> >::type::value == 3,"max_element");

//sort
static_assert(
	std::is_same<
		tinympl::variadic::sort<tinympl::less,std::tuple,int_<4>,int_<3>,int_<8>,int_<10> >::type,
		std::tuple<int_<3>,int_<4>,int_<8>,int_<10> >
	>::type::value,"sort");

//reverse
static_assert(
	std::is_same<
		tinympl::variadic::reverse<std::tuple,int,char,long>::type,
		std::tuple<long,char,int> >::type::value,"reverse");

//unique
static_assert(
	std::is_same<
		tinympl::variadic::unique<std::tuple,int,char,int,long,int>::type,
		std::tuple<int,char,long> >::type::value,"unique");

static_assert(
	std::is_same<
		tinympl::variadic::unique<std::tuple,int,char,long>::type,
		std::tuple<int,char,long> >::type::value,"unique");

//is_unique
static_assert(
	tinympl::variadic::is_unique<int,long,char,double>::type::value == true &&
	tinympl::variadic::is_unique<int,long,int,char>::type::value == false,"is_unique");

//accumulate
static_assert(
	std::is_same<
		tinympl::variadic::accumulate<tinympl::plus,int_<1>,int_<2>,int_<3> >::type,
		int_<6> >::value,"accumulate");

//left_fold
template<class T,class U> struct tuple_push_back {typedef std::tuple<T,U> type;};
template<class T,class ... Args> struct tuple_push_back<std::tuple<Args...>,T> { typedef std::tuple<Args...,T> type; };

static_assert(
	std::is_same<
		tinympl::variadic::left_fold<
			tuple_push_back,
			int,long,char>::type,
		std::tuple<int,long,char>
	>::value,"left_fold");

//right_fold
template<class T,class U> struct tuple_push_front {typedef std::tuple<T,U> type;};
template<class T,class ... Args> struct tuple_push_front<T,std::tuple<Args...> > { typedef std::tuple<T,Args...> type; };

static_assert(
	std::is_same<
		tinympl::variadic::right_fold<
			tuple_push_front,
			int,long,char>::type,
		std::tuple<int,long,char>
	>::value,"right_fold");

//all_of
static_assert(
	tinympl::variadic::all_of<std::is_integral,int,long,short>::type::value == true &&
	tinympl::variadic::all_of<std::is_floating_point,double,int,char>::type::value == false,"all_of");

//any_of
static_assert(
	tinympl::variadic::any_of<std::is_integral,int,long,short>::type::value == true &&
	tinympl::variadic::any_of<std::is_floating_point,long,int,char>::type::value == false,"any_of");

//none_of
static_assert(
	tinympl::variadic::none_of<std::is_integral,double,float,std::string>::type::value == true &&
	tinympl::variadic::none_of<std::is_floating_point,double,int,char>::type::value == false,"none_of");

}
