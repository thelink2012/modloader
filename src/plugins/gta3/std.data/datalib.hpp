/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once

#if _MSC_VER
#   pragma warning(disable : 4018)  // @deps\tinympl\tinympl\variadic\at.hpp(33): warning C4018: '<' : signed/unsigned mismatch
#   pragma warning(disable : 4503)  // decorated name length exceeded, name was truncated
#endif

// datalib fundamentals
#include <datalib/data_slice.hpp>
#include <datalib/dominance.hpp>
#include <datalib/data_store.hpp>

// additional types
#include <datalib/detail/linear_map.hpp>
#include <type_wrapper/floating_point.hpp>
#include <type_wrapper/datalib/io/floating_point.hpp>

// datalib I/O with std types
#include <datalib/io/either.hpp>
#include <datalib/io/tuple.hpp>
#include <datalib/io/array.hpp>
#include <datalib/io/string.hpp>
#include <datalib/io/ignore.hpp>

// datalib gta3 stuff
#include <datalib/gta3/io.hpp>
#include <datalib/gta3/data_section.hpp>
#include <datalib/gta3/data_store.hpp>

using namespace datalib;



//
// Useful types to use on our gta3 data processing
//

using real_t = basic_floating_point<float, floating_point_comparer::relative_epsilon<float>>;

template<std::size_t N>
using vecn = std::array<real_t, N>;
using vec2 = vecn<2>;
using vec3 = vecn<3>;
using vec4 = vecn<4>;
