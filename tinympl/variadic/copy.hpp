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

#ifndef TINYMPL_VARIADIC_COPY_HPP
#define TINYMPL_VARIADIC_COPY_HPP

namespace tinympl {
namespace variadic {

/**
 * \ingroup VarModAlgs
 * \class copy
 * \brief Copy the elements from the input sequence to the output sequence
 * \param Out The output sequence type - defaults to the same sequence kind of
the input sequence
 * \param Args... The input sequence
 * \return `copy<...>::type` is a type templated from `Out` which is constructed
with the elements of SequenceIn.
 * \sa tinympl::copy
 */
template<template<class ...> class Out, class ... Args>
struct copy {
    typedef Out<Args...> type;
};

} // namespace variadic
} // namespace tinympl

#endif // TINYMPL_VARIADIC_COPY_HPP
