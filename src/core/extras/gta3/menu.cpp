/*
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */

#include <cstdint>
#include "../../loader.hpp"
#include <modloader/util/injector.hpp>
#include <injector/calling.hpp>
#include "MenuExtender/MenuExtender.h"

using plugin::MenuExtender;
using namespace modloader;



static CMenuItem static_entries[] = 
{
	{   "FEM_MM", -1, 0,
		    5, "FEP_STG", 11, 1,  300, 200,  3,
		    5, "FEP_OPT", 11, 33, 0,   0,    3,
		    5, "FEP_QUI", 11, 35, 0,   0,    3,
    },
};




void test_menu()
{
    static MenuExtender mm;
    mm.Initialise();

    auto nScreens = std::distance(std::begin(static_entries), std::end(static_entries));

    static BYTE test_action;
    test_action = mm.RegisterAction();

    static CMenuItem* screen;
    screen = mm.RegisterMenuScreen(nScreens);
    memcpy(screen, static_entries, sizeof(static_entries));
    
    if(auto aa = mm.GetEntry(screen, "FEP_QUI"))
        aa->m_nActionType = test_action;

    mm.AddTargetMenu(mm.GetScreen("FEM_MM"), screen, "FES_ACH");

    
    mm.RegisterActionHandler([](const MenuExtender::ActionInfo& info)
    {
        auto screen = mm.GetScreen(info.screen);
        auto action = screen->m_aEntries[info.entry].m_nActionType;
        loader.Log("%d %d\n", action,  test_action);
        if(action == test_action)
        {
            //loader.Error("AEEEEEEEEEEEEE");
            char tmpbuf[68];
            auto a1 = mm.GetEntry(screen, "FEP_STG");
            auto a2 = mm.GetEntry(screen, "FEP_OPT");
            
            strcpy(tmpbuf, a1->m_szName);
            strcpy(a1->m_szName, a2->m_szName);
            strcpy(a2->m_szName, tmpbuf);

            return true;
        }
        return false;
    });

    mm.RegisterBackgroundHandler([](MenuExtender::BackgroundInfo& info)
    {
        if(mm.GetScreen(info.screen) == screen)
        {
            info.sprite = (CSprite2d*)((uintptr_t)(mm.GetMenuManager()) + 0x12C + 0xC);
            return true;
        }
        return true;
    });

    mm.RegisterUserInputHandler([](MenuExtender::UserInputInfo& info)
    {
        if(mm.GetScreen(info.screen) == screen)
        {
            if(info.wheel > 0) loader.Error("YEAJ");
            return true;
        }
        return true;
    });

    mm.RegisterDrawHandler([](MenuExtender::DrawInfo& info)
    {
        if(mm.GetScreen(info.screen) == screen)
        {
            info.drawtitle = false;
            injector::ThisCall<void>(injector::raw_ptr(0x57E240), info.menumgr, "STAT004");
            return true;
        }
        return true;
    });
}
