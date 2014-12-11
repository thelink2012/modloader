
#include <tinympl/ratio.hpp>
#include <tinympl/algorithm.hpp>

using namespace tinympl;

namespace ratio_example
{

typedef std::tuple<
	std::ratio<5,8>,
	std::ratio<4,3>,
	std::ratio<16,8>,
	std::ratio<4,2>,
	std::ratio<2,3>
> seq1;

static_assert(max_element<seq1>::value == 2,"ratio - max element");

static_assert(
	std::is_same<
		sort<seq1>::type,
		std::tuple<
			rational<5,8>,
			rational<2,3>,
			rational<4,3>,
			std::ratio<16,8>,
			std::ratio<4,2>
		>
	>::value,"ratio - sort");

static_assert(
	std::is_same<
		std::ratio<4,2>,
		std::ratio<8,4> >::value == false, "std::ratio compare example");

static_assert(
	equal_to<
		std::ratio<4,2>,
		std::ratio<8,4> >::value == true, "equal_to compare example");

static_assert(
	std::is_same<
		rational<4,2>,
		rational<8,4> >::value == true, "rational compare example");
}
