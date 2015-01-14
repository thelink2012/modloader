
#include <tinympl/vector.hpp>

namespace vector_example
{

typedef tinympl::vector<int,long,char> v1;
	
static_assert( v1::size == 3,"size");
static_assert( std::is_same< v1::at<1>::type,long>::value,"at");

typedef v1::push_back<std::string>::type v2;
static_assert( std::is_same < v2::at<3>::type,std::string>::value,"push_back");

typedef v2::push_front<double>::type v3;
static_assert( std::is_same < v3::at<0>::type,double>::value,"push_front");

typedef v1::pop_back::type v4;
static_assert( std::is_same< v4, tinympl::vector<int,long> >::value,"pop_back");

typedef v4::pop_front::type v5;
static_assert( std::is_same< v5, tinympl::vector<long> >::value,"pop_front");

typedef v1::insert<1,float,float,char>::type v6;
static_assert( std::is_same< v6, tinympl::vector<int,float,float,char,long,char> >::value,"insert");

typedef v6::erase<0,3>::type v7;
static_assert( std::is_same< v7, tinympl::vector<char,long,char> >::value,"erase");

}
