// Copyright (C) 2013, Ennio Barbaro,
// Copyright (C) 2014, Pawel Tomulik.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://sbabbi.github.io/tinympl for documentation.
//
// You are welcome to contact the authors at:
//  enniobarbaro@gmail.com, ptomulik@meil.pw.edu.pl
//

#include <tinympl/test_config.hpp>

#if TINYMPL_VECTOR_TEST_ENABLE

// Order of includes is important!
// First, include tinympl/vector.hpp, then <tinympl/unit_test.hpp>
#include <tinympl/vector.hpp>
#include <tinympl/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_tinympl)
BOOST_AUTO_TEST_SUITE(vector)

BOOST_AUTO_TEST_CASE(always_pass)
{
  BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif // TINYMPL_VECTOR_TEST_ENABLED
