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

#ifndef TINYMPL_TRANSFORM_MANY_HPP
#define TINYMPL_TRANSFORM_MANY_HPP

#include <tinympl/transform.hpp>
#include <tinympl/transpose.hpp>
#include <tinympl/copy.hpp>
#include <tinympl/sequence.hpp>

namespace tinympl {

/**
 * \ingroup SeqModAlgs
 * \class transform_many
 * \brief Transform many input sequences using a function
 * \param F The transform function. `F<Args...>::type`, where
`size<Args...>::value` is the number of input sequences, must be a valid
expression.
 * \param Out The output sequence type
 * \param Sequences... The input sequences
 * \return `transform_many<...>::type` is a type templated from `Out` which
contains the transformed types
 */
template<template<class ...> class F,
        template<class ...> class Out,
        class ... Sequences>
struct transform_many {
    template<class Seq> using F_t = typename copy<Seq, F>::type::type;

    typedef typename transform <
    typename transpose< sequence<Sequences...>, sequence, sequence>::type,
             F_t,
             Out >::type type;
};

} // namespace tinympl

#endif // TINYMPL_TRANSFORM_MANY_HPP
