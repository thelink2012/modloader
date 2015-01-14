
#include <type_traits>
#include <tinympl/string.hpp>
#include <tinympl/string_macro.hpp>
#include <tinympl/to_string.hpp>

namespace test_string
{

using namespace tinympl;

typedef basic_string<char,'l','o','w','d'> str_lowd;
typedef str_lowd::insert_c<0,'h','e','l'>::type str_hellowd;
typedef str_hellowd::insert_c<6,'o','r','l'>::type str_helloworld;

static_assert(
	std::is_same<
	str_helloworld,
		basic_string<char,'h','e','l','l','o','w','o','r','l','d'>
	>::value, "basic_string::insert_c ");

static_assert(
	basic_string<char,'h','e','l','l'>::compare<
		basic_string<char,'h','e','l','l','o'>
	>::value == -1,"basic_string::compare");

static_assert(
	basic_string<char,'h','f','l','l','o'>::compare<
		basic_string<char,'h','e','l','l','o'>
	>::value == 1,"basic_string::compare");

static_assert(
	basic_string<char,'h','e','l','l','o'>::compare<
		basic_string<char,'h','e','l','l','o'>
	>::value == 0,"basic_string::compare");

constexpr char HelloWorld[] = "helloworld";

static_assert(
	std::is_same<
		string<HelloWorld>,
		str_helloworld
	>::value,"string<>");

MAKE_TINYMPL_STRING(str_helloworld2,"helloworld");

static_assert(
	std::is_same<
		str_helloworld2,
		str_helloworld
	>::value,"make_mpl_string");

MAKE_TINYMPL_STRING(str_lowor,"lowor");
static_assert(
	std::is_same<
		str_lowor,
		str_helloworld::substr<3,5>::type
	>::value,"basic_string::substr");

MAKE_TINYMPL_STRING(str_ZZZ,"ZZZ");
MAKE_TINYMPL_STRING(str_helZZZworld,"helZZZworld");

static_assert(
	std::is_same<
		str_helloworld::replace<3,2,str_ZZZ>::type,
		str_helZZZworld>::value,"basic_string::replace");
		

MAKE_TINYMPL_STRING(str_world,"world");

static_assert(str_helloworld::find<str_world>::value == 5,"string::find");
static_assert(str_lowor::find<str_world>::value == 5,"string::find");


MAKE_TINYMPL_STRING(str_minus243,"-243");

static_assert(
	std::is_same<
		str_minus243,
		to_string_i_t<-243>
	>::value,"to_string");

}
