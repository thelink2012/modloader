/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Game ranges
 * 
 * 
 */
#ifndef TRAITS_GAME_RANGES_H
#define	TRAITS_GAME_RANGES_H

#include <modloader_util_container.hpp>

/*
 *  Nothing here was checked in the game executable, must make sure about those values later
 */

inline bool test_range(int id, int min, int max)
{
    return !(id < min || id > max);
}

inline bool is_ped(int id)
{
    return test_range(id, 0, 300);
}

inline bool is_vehicle(int id)
{
    return test_range(id, 400, 615);
}

inline bool is_weapon(int id)
{
    return test_range(id, 320, 370);
}

inline bool is_object(int id)
{
    return id > 1000 || (!is_ped(id) && !is_vehicle(id) && !is_weapon(id));
}


/*
 *  Checks if @modelname is a vehicle upgrade model 
 */
inline bool IsUpgradeModel(const char* modelname)
{
    static const char* uptable[] =
    {
        "wheel_", "nto_", "exh_", "bnt_", "chss_", "bntl_", "bntr_", "spl_",
        "hydralics", "stereo", "wg_l_", "wg_r_", "lgt_",
        "fbb_", "bbb_", "rf_", "fbmp_", "rbmp_",
        "misc_a_", "misc_b_", "misc_c_",
        nullptr
    };
    
    for(const char** p = uptable; *p; ++p)
    {
        if(modloader::starts_with(modelname, *p, false))
            return true;
    }
    
    return false;
 }

#endif

