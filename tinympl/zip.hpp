// Copyright (C) 2013, Ennio Barbaro.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://sbabbi.github.io/tinympl for documentation.
//
// You are welcome to contact the author at:
//  enniobarbaro@gmail.com
//

#ifndef TINYMPL_ZIP_HPP
#define TINYMPL_ZIP_HPP

#include <tinympl/transpose.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class zip
 * \brief Construct a single sequence by zipping together multiple sequences
 * \param OutSequence The type of the output sequence
 * \param ZipType The type of the inner sequence
 * \param Sequences... The input sequences
 */
template<template<class ...> class OutSequence,
        template<class ...> class ZipType,
        class ... Sequences>
struct zip : transpose< sequence<Sequences...>, OutSequence, ZipType> {};

} // namespace tinympl

#endif // TINYMPL_ZIP_HPP
