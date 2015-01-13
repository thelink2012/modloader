/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#pragma warning(disable : 4503)     // 'identifier' : decorated name length exceeded, name was truncated
#define BOOST_OPTIONAL_NO_IOFWD
#undef INJECTOR_GVM_HAS_TRANSLATOR  // TODO make it use the translator
#include <stdinc/gta3/stdinc.hpp>
#include <regex/regex.hpp>          // Abstraction over boost::xpressive or std::regex
#include <regex/fregex.hpp>
#include <boost/optional.hpp>
#include "datalib.hpp"
#include "utility.hpp"

// We are going to use lots of type erasure
#include <typeinfo>
#include <typeindex>
#include <boost/any.hpp>

using boost::any;

template<class T>
using maybe = boost::optional<T>;                   // alias for more expressive circumstances
static const boost::none_t nothing = boost::none;   // more haskell like none type


// data.hpp and cache.hpp should not be included over here
// see main.cpp for details on why


