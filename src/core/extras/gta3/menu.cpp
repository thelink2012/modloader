/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 *      Mod Loader In-Game Menu
 *
 */
#include <memory>
#include "../../loader.hpp"
#include "menu.hpp"
#include <modloader/util/injector.hpp>
#include <modloader/util/fxt.hpp>
using namespace modloader;


static CMenuItem StaticPages[] = 
{
    // Action, Name, Type, TargetMenu, RX, RY, Align

    // Main Page
	{   "_ML_0HH", -1, 0,
            MENU_ACTION_DUMMY,      "_ML_0E0",      MENU_ENTRY_OPTION,    -1,  57, 100,  1,
            MENU_ACTION_DUMMY,      "_ML_0EP",      MENU_ENTRY_OPTION,    -1,   0,   0,  1,
            MENU_ACTION_DUMMY,      "_ML_0EL",      MENU_ENTRY_OPTION,    -1,   0,   0,  1,
            MENU_ACTION_DUMMY,      "_ML_0LF",      MENU_ENTRY_OPTION,    -1,   0,   0,  1,
            MENU_ACTION_DUMMY,      "_ML_0LS",      MENU_ENTRY_OPTION,    -1,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_0MM",      MENU_ENTRY_BUTTON,     1,   0,   0,  1,
		    
            MENU_ACTION_BACK,       "_ML_FTB",      MENU_ENTRY_BUTTON,    -1, 490, 380,  1,
    },
    
    // Modifications Page
	{   "_ML_MHH", 0, 0,
            MENU_ACTION_SWITCH,     "_ML_MY1",      MENU_ENTRY_DUMMY,      2,  30,  90,  1,
            MENU_ACTION_SWITCH,     "_ML_MY2",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_MY3",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_MY4",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_MY5",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_MY6",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_MY7",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,
            MENU_ACTION_SWITCH,     "_ML_MY8",      MENU_ENTRY_DUMMY,      2,   0,   0,  1, // TODO MENU_ENTRY_BUTTON \/\ --
            MENU_ACTION_SWITCH,     "_ML_MY9",      MENU_ENTRY_DUMMY,      2,   0,   0,  1,

            MENU_ACTION_SWITCH,     "_ML_FTP",      MENU_ENTRY_BUTTON,    -1,  30, 380,  1,
            MENU_ACTION_BACK,       "_ML_FTB",      MENU_ENTRY_BUTTON,    -1, 320, 380,  3,
            MENU_ACTION_SWITCH,     "_ML_FTN",      MENU_ENTRY_BUTTON,    -1, 500, 380,  1,
    },

    // Selected Modification Page
	{   "_ML_YHH", 1, 0,
            MENU_ACTION_DUMMY,      "_ML_YE0",      MENU_ENTRY_DUMMY,     -1,  30,  90,  1,
            MENU_ACTION_DUMMY,      "_ML_YIC",      MENU_ENTRY_DUMMY,     -1,   0,   0,  1,
            MENU_ACTION_DUMMY,      "_ML_YPR",      MENU_ENTRY_DUMMY,     -1,   0,   0,  1,
            MENU_ACTION_DUMMY,      "_ML_YIX",      MENU_ENTRY_DUMMY,     -1,   0,   0,  1,
            MENU_ACTION_DUMMY,      "_ML_YSS",      MENU_ENTRY_DUMMY,     -1,   0,   0,  1,

            MENU_ACTION_BACK,       "_ML_FTB",      MENU_ENTRY_BUTTON,    -1, 490, 380,  1,
    },

};

//
static const auto NumStaticPages = std::extent<decltype(StaticPages)>::value;
static const auto NumPages = NumStaticPages;
static const auto NumModsPerPage = 9;


class TheMenu : public AbstractFrontend
{
    private:
        fxt_manager fxt;

        CMenuItem*      pOptions;       // The options menu page
        CMenuEntryData* pEntry;         // The entry at the options menu we are in

        CMenuItem*      pPages;         // Our allocated pages array
        MenuPage        mPageMain;      // The "Mod Loader" page
        MenuPage        mPageMods;      // The "Modifications" page
        MenuPage        mPageMod;       // The "The Mod Config" page

    public:
        TheMenu();

    private:
        void LoadText();
        void CreateMenu();
        void RegisterEvents();

        void MainPageEvents();
        void ModsPageEvents();
        void ModPageEvents();

        static void SaveBasicConfig(MenuEntry&);
};

TheMenu::TheMenu()
{
    this->LoadText();
    this->CreateMenu();
    this->RegisterEvents();
}

void TheMenu::LoadText()
{
    // TODO multi-language
    if(!ParseFXT(fxt, (loader.dataPath + "/text/test_menu.fxt").data()))
    {
    }
}

void TheMenu::CreateMenu()
{
    // Allocate and setup array of pages
    this->pPages = RegisterMenuPage(NumStaticPages);
    std::memcpy(pPages, StaticPages, sizeof(StaticPages));
    this->RelativeSetup(pPages, NumStaticPages);

    // 
    this->mPageMain.Initialise(GetPage("_ML_0HH", pPages, NumPages));
    this->mPageMods.Initialise(GetPage("_ML_MHH", pPages, NumPages));
    this->mPageMod.Initialise(GetPage("_ML_YHH", pPages, NumPages));

    // Add our menu after the Options menu
    this->pOptions = GetPage("FET_OPT");
    this->pEntry = AddTargetMenuBefore(pOptions, GetEntry(pOptions, "FEDS_TB"), this->mPageMain, "_ML_FEO");
}

void TheMenu::RegisterEvents()
{
    this->MainPageEvents();
    this->ModsPageEvents();
    this->ModPageEvents();
}

void TheMenu::SaveBasicConfig(MenuEntry&)
{
    loader.SaveBasicConfig();
}

void TheMenu::MainPageEvents()
{
    auto SetupBooleanEntry = [this](const char* label, bool& boolean)
    {
        if(auto entry = mPageMain.GetEntry(label))
        {
            entry->OnStatefulChange(SaveBasicConfig);
            OnAction(this->pOptions, this->pEntry, entry->SetupBooleanEntry(std::ref(boolean)));
        }
    };
    
    auto SetupMegaByteEntry = [this](const char* label, uint64_t& size)
    {
        static const uint64_t mb = 1048576;
        static const uint64_t min = mb, max = mb * 100, step = mb;

        static auto MegaByteLabel = [](const uint64_t& bytes)
        {
            auto text = NumberLabel(bytes / mb);
            text.first.append("MB");
            return text;
        };

        if(auto entry = mPageMain.GetEntry(label))
        {
            entry->OnStatefulChange(SaveBasicConfig);

            auto fun = entry->SetupStatefulEntry(std::ref(size), std::ref(MegaByteLabel), [](const uint64_t& value, ActionInfo& info)
            {
                return std::min(std::max((value + step * info.wheel), min), max);
            });

            OnAction(this->pOptions, this->pEntry, std::move(fun));
        }
    };

    SetupBooleanEntry("_ML_0E0", loader.bEnableMenu);
    SetupBooleanEntry("_ML_0EP", loader.bEnablePlugins);
    SetupBooleanEntry("_ML_0EL", loader.bEnableLog);
    SetupBooleanEntry("_ML_0LF", loader.bImmediateFlush);
    SetupMegaByteEntry("_ML_0LS", loader.maxBytesInLog);

    // _ML_0MM is handled at ModsPageEvents
}

void TheMenu::ModsPageEvents()
{
    auto pButtonNext = mPageMods.GetEntry("_ML_FTN");
    auto pButtonPrev = mPageMods.GetEntry("_ML_FTP");

    OnAction(this->mPageMain, *mPageMain.GetEntry("_ML_0MM"), [this](ActionInfo& info)
    {
        // TODO FirstPage()
        return true;
    });

    OnUserInput(this->mPageMods, [this](UserInputInfo& info)
    {
        // TODO NextPage(), PrevPage()
        return true;
    });

    OnAction(this->mPageMods, MENU_ACTION_SWITCH, [this, pButtonNext, pButtonPrev](ActionInfo& info)
    {
        // TODO IF pButtonNext|Prev NextPage(), PrevPage()
        return true;
    });

    // _ML_MY1 is handled at ModPageEvents
}

void TheMenu::ModPageEvents()
{
    auto EntryFromIndex = [this](int i)
    {
        return mPageMods.GetEntry(std::string("_ML_MY").append(std::to_string(i)).data());
    };

    for(int i = 1; auto entry = EntryFromIndex(i); ++i)
    {
        OnAction(this->mPageMods, *entry, [this, i](ActionInfo& info)
        {
            // TODO BuildModPage()
            return true;
        });
    }


    OnAction(this->mPageMod, *mPageMod.GetEntry("_ML_YE0"), [](ActionInfo& info)
    {
        // TODO
        return true;
    });

    OnAction(this->mPageMod, *mPageMod.GetEntry("_ML_YIC"), [](ActionInfo& info)
    {
        // TODO
        return true;
    });

    OnAction(this->mPageMod, *mPageMod.GetEntry("_ML_YPR"), [](ActionInfo& info)
    {
        // TODO
        return true;
    });

    OnAction(this->mPageMod, *mPageMod.GetEntry("_ML_YIX"), [](ActionInfo& info)
    {
        // TODO Install()/Uninstall()
        return true;
    });

    OnAction(this->mPageMod, *mPageMod.GetEntry("_ML_YSS"), [](ActionInfo& info)
    {
        // TODO Rescan()
        return true;
    });
}





















void test_menu()
{
    static std::unique_ptr<TheMenu> menu;
    menu.reset(new TheMenu());
}
