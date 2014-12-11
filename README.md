tinympl
=======

[![Build Status](https://travis-ci.org/sbabbi/tinympl.svg?branch=master)](https://travis-ci.org/sbabbi/tinympl)

**Tinympl** implements some useful C++ template meta-programming algorithms. 
Most of this work is inspired by [Boost.MPL](http://www.boost.org/doc/libs/1_55_0/libs/mpl/doc/index.html).
Unlike Boost.MPL, this package makes use of the C++11 features, resulting in a much smaller code base.

Main features:

 * Extensive algorithm library which works both with variadic templates and with (possibly user-defined) template types (like `std::tuple`).
 * Compile-time containers, like `map`,  `vector` and `string`.
 * Mixed (compile-time / real-time) `map` types.

[Full doxygen documentation](http://sbabbi.github.io/tinympl/).
