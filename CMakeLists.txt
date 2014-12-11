
cmake_minimum_required(VERSION 2.6)

project(tinympl)
enable_testing()

set(TINYMPL_MAJOR_VERSION 0)
set(TINYMPL_MINOR_VERSION 2)
set(TINYMPL_VERSION ${TINYMPL_MAJOR_VERSION}.${TINYMPL_MINOR_VERSION})

set(TINYMPL_HEADERS
       tinympl/accumulate.hpp
       tinympl/algorithm.hpp
       tinympl/algorithm_variadic.hpp
       tinympl/all_of.hpp
       tinympl/any_of.hpp
       tinympl/apply.hpp
       tinympl/at.hpp
       tinympl/bind.hpp
       tinympl/copy.hpp
       tinympl/copy_if.hpp
       tinympl/copy_n.hpp
       tinympl/count.hpp
       tinympl/count_if.hpp
       tinympl/erase.hpp
       tinympl/fill_n.hpp
       tinympl/find.hpp
       tinympl/find_if.hpp
       tinympl/functional.hpp
       tinympl/fused_map.hpp
       tinympl/fused_value_map.hpp
       tinympl/generate_n.hpp
       tinympl/insert.hpp
       tinympl/is_unique.hpp
       tinympl/join.hpp
       tinympl/lambda.hpp
       tinympl/left_fold.hpp
       tinympl/lexicographical_compare.hpp
       tinympl/map.hpp
       tinympl/max_element.hpp
       tinympl/min_element.hpp
       tinympl/none_of.hpp
       tinympl/ratio.hpp
       tinympl/remove.hpp
       tinympl/remove_if.hpp
       tinympl/replace.hpp
       tinympl/replace_if.hpp
       tinympl/reverse.hpp
       tinympl/right_fold.hpp
       tinympl/sequence.hpp
       tinympl/set_difference.hpp
       tinympl/set_intersection.hpp
       tinympl/set_union.hpp
       tinympl/size.hpp
       tinympl/sort.hpp
       tinympl/string.hpp
       tinympl/string_macro.hpp
       tinympl/to_string.hpp
       tinympl/transform.hpp
       tinympl/transform2.hpp
       tinympl/transform_many.hpp
       tinympl/transpose.hpp
       tinympl/unique.hpp
       tinympl/unordered_equal.hpp
       tinympl/value_map.hpp
       tinympl/variadic.hpp
       tinympl/vector.hpp
       tinympl/zip.hpp)

add_library(tinympl STATIC ${TINYMPL_HEADERS})
set_target_properties(tinympl PROPERTIES LINKER_LANGUAGE CXX)

configure_file(Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)

install(DIRECTORY tinympl DESTINATION include)

add_definitions(-std=c++11)
add_definitions(-Wall -pedantic)

add_executable(tinympl_test examples/main.cpp)
set_target_properties(tinympl_test PROPERTIES INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})

add_executable(test/test_tinympl
       test/test_tinympl.cpp
       test/tinympl/accumulate.cpp
       test/tinympl/algorithm.cpp
       test/tinympl/algorithm_variadic.cpp
       test/tinympl/all_of.cpp
       test/tinympl/and_b.cpp
       test/tinympl/any_of.cpp
       test/tinympl/as_sequence.cpp
       test/tinympl/apply.cpp
       test/tinympl/at.cpp
       test/tinympl/bind.cpp
       test/tinympl/bool.cpp
       test/tinympl/char.cpp
       test/tinympl/copy.cpp
       test/tinympl/copy_if.cpp
       test/tinympl/copy_n.cpp
       test/tinympl/count.cpp
       test/tinympl/count_if.cpp
       test/tinympl/divides.cpp
       test/tinympl/equal_to.cpp
       test/tinympl/erase.cpp
       test/tinympl/fill_n.cpp
       test/tinympl/find.cpp
       test/tinympl/find_if.cpp
       test/tinympl/functional.cpp
       test/tinympl/fused_map.cpp
       test/tinympl/fused_value_map.cpp
       test/tinympl/generate_n.cpp
       test/tinympl/greater.cpp
       test/tinympl/greater_equal.cpp
       test/tinympl/identity.cpp
       test/tinympl/if.cpp
       test/tinympl/inherit.cpp
       test/tinympl/insert.cpp
       test/tinympl/int.cpp
       test/tinympl/is_sequence.cpp
       test/tinympl/is_unique.cpp
       test/tinympl/join.cpp
       test/tinympl/lambda.cpp
       test/tinympl/left_fold.cpp
       test/tinympl/less.cpp
       test/tinympl/less_equal.cpp
       test/tinympl/lexicographical_compare.cpp
       test/tinympl/logical_and.cpp
       test/tinympl/logical_not.cpp
       test/tinympl/logical_or.cpp
       test/tinympl/long.cpp
       test/tinympl/map.cpp
       test/tinympl/max_element.cpp
       test/tinympl/min_element.cpp
       test/tinympl/minus.cpp
       test/tinympl/modulus.cpp
       test/tinympl/multiplies.cpp
       test/tinympl/negate.cpp
       test/tinympl/none_of.cpp
       test/tinympl/not_b.cpp
       test/tinympl/not_equal_to.cpp
       test/tinympl/or_b.cpp
       test/tinympl/plus.cpp
       test/tinympl/ratio.cpp
       test/tinympl/remove.cpp
       test/tinympl/remove_if.cpp
       test/tinympl/replace.cpp
       test/tinympl/replace_if.cpp
       test/tinympl/reverse.cpp
       test/tinympl/right_fold.cpp
       test/tinympl/sequence.cpp
       test/tinympl/set_difference.cpp
       test/tinympl/set_intersection.cpp
       test/tinympl/set_union.cpp
       test/tinympl/short.cpp
       test/tinympl/size.cpp
       test/tinympl/sizeof.cpp
       test/tinympl/sort.cpp
       test/tinympl/string.cpp
       test/tinympl/string_macro.cpp
       test/tinympl/to_string.cpp
       test/tinympl/transform.cpp
       test/tinympl/transform2.cpp
       test/tinympl/transform_many.cpp
       test/tinympl/transpose.cpp
       test/tinympl/unique.cpp
       test/tinympl/unordered_equal.cpp
       test/tinympl/value_map.cpp
       test/tinympl/variadic.cpp
       test/tinympl/variadic/accumulate.cpp
       test/tinympl/variadic/all_of.cpp
       test/tinympl/variadic/any_of.cpp
       test/tinympl/variadic/at.cpp
       test/tinympl/variadic/copy.cpp
       test/tinympl/variadic/copy_if.cpp
       test/tinympl/variadic/copy_n.cpp
       test/tinympl/variadic/count.cpp
       test/tinympl/variadic/count_if.cpp
       test/tinympl/variadic/erase.cpp
       test/tinympl/variadic/fill_n.cpp
       test/tinympl/variadic/find.cpp
       test/tinympl/variadic/find_if.cpp
       test/tinympl/variadic/generate_n.cpp
       test/tinympl/variadic/insert.cpp
       test/tinympl/variadic/is_unique.cpp
       test/tinympl/variadic/left_fold.cpp
       test/tinympl/variadic/max_element.cpp
       test/tinympl/variadic/min_element.cpp
       test/tinympl/variadic/none_of.cpp
       test/tinympl/variadic/remove.cpp
       test/tinympl/variadic/remove_if.cpp
       test/tinympl/variadic/replace.cpp
       test/tinympl/variadic/replace_if.cpp
       test/tinympl/variadic/reverse.cpp
       test/tinympl/variadic/right_fold.cpp
       test/tinympl/variadic/size.cpp
       test/tinympl/variadic/sort.cpp
       test/tinympl/variadic/transform.cpp
       test/tinympl/variadic/unique.cpp
       test/tinympl/vector.cpp
       test/tinympl/zip.cpp)

target_include_directories(test/test_tinympl
       PUBLIC ${CMAKE_SOURCE_DIR}
       PUBLIC ${CMAKE_SOURCE_DIR}/test
)

target_link_libraries(test/test_tinympl boost_unit_test_framework)
set_target_properties(test/test_tinympl PROPERTIES COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK=1)

add_test( tinympl_runtime tinympl_test )
add_test( new_tinympl_runtime test/test_tinympl )
