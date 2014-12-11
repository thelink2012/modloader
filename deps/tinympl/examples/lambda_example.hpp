
#include <tinympl/lambda.hpp>
#include <vector>

namespace test_lambda
{

using namespace tinympl;

template<class T> using myf = lambda< 
	if_< 
		bind<std::is_same, arg1,int_<5> >, // 	equal_to< int_<0>, modulus<arg1, int_<5> > >,
		plus< arg1, int_<8> >,
		multiplies< arg1, int_<5> > >
	>::template eval<T>;

static_assert( myf<int_<4> >::type::value == 20,"lambda");
static_assert( myf<int_<5> >::type::value == 13,"lambda");

template<bool b,class U> using myf2 = lambda<
	if_< arg1,
		protect<std::vector<arg2> >,
		protect<std::vector<int> > >
	>::template eval_t<bool_<b>,U>;

static_assert( std::is_same<
					myf2<true, char>,
					std::vector<char>
				>::value &&
				std::is_same<
					myf2<false,char>,
					std::vector<int>
				>::value,"lambda");
}
