/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once

#if _MSC_VER
#   pragma warning(disable : 4018)  // @deps\tinympl\tinympl\variadic\at.hpp(33): warning C4018: '<' : signed/unsigned mismatch
#endif

// datalib fundamentals
#include <datalib/data_slice.hpp>
#include <datalib/dominance.hpp>
#include <datalib/data_store.hpp>

// additional types
#include <datalib/detail/linear_map.hpp>
#include <type_wrapper/floating_point.hpp>
#include <type_wrapper/datalib/io/floating_point.hpp>

// datalib I/O with common types
#include <datalib/io/enum.hpp>
#include <datalib/io/either.hpp>
#include <datalib/io/optional.hpp>
#include <datalib/io/tuple.hpp>
#include <datalib/io/pair.hpp>
#include <datalib/io/array.hpp>
#include <datalib/io/string.hpp>
#include <datalib/io/ignore.hpp>
#include <datalib/io/hex.hpp>
#include <datalib/io/tagged_type.hpp>
#include <datalib/io/udata.hpp>
#include <datalib/io/dyncontainer.hpp>
#include <datalib/data_info/vector.hpp>
#include <datalib/data_info/list.hpp>
#include <datalib/data_info/set.hpp>
#include <datalib/data_info/deque.hpp>

// datalib gta3 stuff
#include <datalib/gta3/io.hpp>
#include <datalib/gta3/data_section.hpp>
#include <datalib/gta3/data_store.hpp>

using namespace datalib;
