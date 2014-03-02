/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Arguments Translation System 
 *      [*] Redirects ASI system calls that uses a path as input and translates the path to it's actual folder
 *      [*] Make CLEO.asi load cleo scripts inside modloader
 * 
 *  This file should be inlined in asi.cpp
 */

#ifndef ARGS_TRANSLATOR_HPP
#define	ARGS_TRANSLATOR_HPP

// Basic defs
#include "xtranslator.hpp"

// Really specific hacks injected into ASIs to make them work properly
#include "hacks/RyosukeSetModule.hpp"       // For ryosuke plugins
#include "hacks/FindCleoScripts.hpp"        // To load *.cs scripts

// Calling convenitions
#include "translator_cdecl.hpp"             // Argument translator for cdecl funcs
#include "translator_stdcall.hpp"           // Argument translator for stdcall funcs

// Additions
#include "xtranslator_path.hpp"             // Path translators

#endif
