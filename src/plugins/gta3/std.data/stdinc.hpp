/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#pragma once
#pragma warning(disable : 4503)     // 'identifier' : decorated name length exceeded, name was truncated
#define BOOST_OPTIONAL_NO_IOFWD
#include <stdinc/gta3/stdinc.hpp>
#include <regex/regex.hpp>          // Abstraction over boost::xpressive or std::regex
#include <regex/fregex.hpp>
#include <boost/optional.hpp>

// We are going to use lots of type erasure
#include <typeinfo>
#include <typeindex>
#include <boost/any.hpp>
using boost::any;

//
template<class T>
using maybe = boost::optional<T>;
static const boost::none_t nothing = boost::none;

// Our stuff should come at the end (dependencies)
#include "data.hpp"
#include "cache.hpp"
#include "datalib.hpp"
#include "utility.hpp"
