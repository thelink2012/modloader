/*
 * Copyright (C) 2013  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *  Game ranges
 * 
 * 
 */
#ifndef TRAITS_GAME_RANGES_H
#define	TRAITS_GAME_RANGES_H

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

#endif

