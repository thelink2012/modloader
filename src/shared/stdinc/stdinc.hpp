#pragma once

// msvc stuff
#undef  _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#undef  _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

// windows.h stuff
#undef  NOMINMAX
#define NOMINMAX
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

// Commonly used standard C library
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cassert>

// Commonly used standard C++ libraries
#include <map>
#include <set>
#include <list>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <type_traits>

// Mod Loader stuff
#include <modloader/modloader.hpp>
#include <modloader/utility.hpp>
#include <modloader/util/injector.hpp>
#include <modloader/util/container.hpp>
#include <modloader/util/path.hpp>
#include <modloader/util/hash.hpp>
#include <modloader/util/detour.hpp>
