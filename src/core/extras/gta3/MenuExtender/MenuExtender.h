/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under zlib license, see LICENSE at top level directory.
 *
 *  Menu Extender, See Below For More Details
 *
 */
#pragma once
#include <functional>

// Forward, do not include the files directly
struct MenuExtenderData;        // From SharedData
class CMenuManager;
class CSprite2d;
class CRect;
class CRGBA;
class CText;
struct CMenuItem;
struct CMenuEntryData;


/*******************************************************************************/
/****** OKAY, SO, WE'RE NOT ON PLUGIN-SDK ENVIROMENT, LET'S DECLARE THOSE HERE */

// TODO MOVE THIS TO CMenuManager

enum eMenuActions   // There's many actions @0x57702E and @0x57CD88
{
    MENU_ACTION_NONE     = 0,
    MENU_ACTION_TEXT     = 1,       // Some static text at the top of the page (works only on first entry)
    MENU_ACTION_BACK     = 2,       // Back to previous menu
    MENU_ACTION_YES      = 3,       // Used as YES in menus (also as NO, weird?)
    MENU_ACTION_NO       = 4,       // Used as NO in menus  (also as YES, weird?)
    MENU_ACTION_SWITCH   = 5,       // Switch to target menu
    MENU_ACTION_SKIP     = 20,      // Skip this entry (unselectable)
    MENU_ACTION_BACK_PC  = 55,      // Same as BACK without a extra checking (?)
};

enum eMenuEntryType
{
	MENU_ENTRY_NONE,
	MENU_ENTRY_SAVE_1,
	MENU_ENTRY_SAVE_2,
	MENU_ENTRY_SAVE_3,
	MENU_ENTRY_SAVE_4,
	MENU_ENTRY_SAVE_5,
	MENU_ENTRY_SAVE_6,
	MENU_ENTRY_SAVE_7,
	MENU_ENTRY_SAVE_8,
	MENU_ENTRY_MISSIONPACK,
	MENU_ENTRY_JOYMOUSE,
	MENU_ENTRY_BUTTON,
	MENU_ENTRY_OPTION,
};

class CRGBA { public: unsigned int rgba; };

class CRect { public: 
	float m_fLeft;
	float m_fBottom;
	float m_fRight;
	float m_fTop;
};

#pragma pack(push, 4)
struct CMenuEntryData
{
  unsigned char m_nActionType;
  char m_szName[8];
  char m_nType;
  char m_nTargetMenu;
  //char _pad1;
  __int16 m_wPosnX;
  __int16 m_wPosnY;
  char m_nAlign;
  //char _pad2;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct CMenuItem
{
  char m_szTitleName[8];
  char m_nPrevMenu;
  char m_nStartingEntry;
  CMenuEntryData m_aEntries[12];
};
#pragma pack(pop)

static_assert(sizeof(CMenuEntryData) == 0x12, "...");
static_assert(sizeof(CMenuItem) == 0xE2, "...");
static_assert(sizeof(CRGBA) == 0x4, "...");
/*******************************************************************************/


namespace plugin {

/**
    MenuExtender
        This class can extend easily extend the game menu content.
        Instantiate this class ONLY ONCE!! Call Initialise() before using.
        Feel free to derive from this to build your menu pieces.

        What does this class provide?
            [*] Methods to register and create new menu pages
            [*] Methods to register and create new menu actions
            [*] Methods to register and create new menu entry type (unsure about this concept)
            [*] Methods to easily access and modify game menu pages
            [*] Events to easily work and modify the game menu behaviour without any effort to place a hook
                    [*] Action Event        -- Here you can handle any action any menu entry triggers
                    [*] User Input Event    -- Here you can handle user inputs (up, down, enter, esc, left/right)
                    [*] Background Event    -- Here you can change the background sprite, rect and color of any menu
                    [*] Drawing Event       -- Here you can handle a special drawing for an menu, draw a brief text or any other kind of drawing


        Some TIPS to save menu indices, action indices and type indices:
            [*] Register only the really amount of necessary pages, remember you can modify pages on the fly!
            [*] Do not register new actions... Yeah, for what? On the Action Event Handler here you can just check if the page and entry are yours!
            [*] Do not register new types... Same reason as above
            [*] As said above to not register action/types, you could use the MenuExtender::special_action and MenuExtender::special_type to use as dummy
                to redirect to your pointer checking
*/
class MenuExtender
{
    public:
        static const int max_entries = 12;      // Max entries in a menu page
        static const int max_pages = 127;       // Max possible menu pages
        static const int max_action = 255 - 1;  // (one of them is for dummyness)
        static const int max_types   = 127 - 1; // (^^^)
        static const int action_goto_page = 5;
        static const int type_goto_page = 11;

        static const int dummy_action = max_action + 1;
        static const int dummy_type   = max_types + 1;

    protected:
        MenuExtenderData* block;
        bool initialized;

    public:
        MenuExtender() : block(0), initialized(false)
        {}

        // This class can be extended deeply
        virtual ~MenuExtender()
        {}


        /**
         *  Initialises the menu extender.
         *   
         *  Why not on constructor? Because exe may be encrypted when asi gets loaded with an static MenuExtender being constructed
         *  Please note you should initialise ONLY ONE MenuExtender EVER in a single plugin.
         */
        void Initialise();



        /**
         *  Registers new menu pages
         *  @param quantity     The quantity of pages to regiser
         *  @return             Returns an array of pages on null on failure
         */ 
        CMenuItem* RegisterMenuPage(int quantity = 1);

        /**
         *  Registers entry actions
         *  @param quantity     The quantity of actions to regiser
         *  @return             The first action registered or -1 on failure
         */
        int RegisterAction(int quantity = 1);

        /**
         *  Registers entry types
         *  @param quantity     The quantity of types to regiser
         *  @return             The first type registered or -1 on failure
         */
        int RegisterEntryType(int quantity = 1);



        /**
         *  Returns the pointer to the menu page at the specified index
         *  @param i            The index to retrieve the page from
         *  @return             The page at the specified index or null on failure
         */
        CMenuItem* GetPage(int i = 0);

        /**
         *  Returns the index from the specified menu page
         *  @param page         The page to retrieve the index from
         *  @return             The page index or -1 on failure
         */
        int GetPageIndex(const CMenuItem* page);

        /**
         *  Gets a page with the specified title
         *  @param title        The page title to find
         *  @return             The page with the specified title or null if page hasn't been found
         */
        CMenuItem* GetPage(const char* title);

        /**
         *  Gets a page with the specified title
         *  @param title        The page title to find
         *  @param pages        The pages array to find in
         *  @param npages       The number of pages in the array to find in
         */
        CMenuItem* GetPage(const char* title, CMenuItem* pages, size_t npages);


        /**
         *  Adds a new entry into the specified menu page pointing to another menu
         *  @param page     The page to add the entry into
         *  @param target   The target page the entry should point to
         *  @return         Returns the newly created entry or null on failure or if page or target are null
         */
        CMenuEntryData* AddTargetMenu(CMenuItem* page, CMenuItem* target, const char* text);

        /**
         *  Adds a new entry into the specified menu page pointing to another menu.
         *  Be careful with this function, it will invalidate any index/pointer to the 'before' entry and the entries following it
         *  @param page     The page to add the entry into
         *  @param before   The new entry will be created before this entry
         *  @param target   The target page the entry should point to
         *  @return         Returns the newly created entry or null on failure or if page, target or before are null
         */
        CMenuEntryData* AddTargetMenuBefore(CMenuItem* page, CMenuEntryData* before, CMenuItem* target, const char* text);

        /**
         *  Sets the target menu page for the specified menu entry
         *  @param entry    The entry to change the target page
         *  @param target   The target page
         *  @param text     An gxt entry for entry or null to not change it's gxt entry
         *  @param prev     Optional previous page for the target page
         *  @return         Returns entry or null on failure or if entry is null
         */
        CMenuEntryData* SetTargetMenu(CMenuEntryData* entry, CMenuItem* target, const char* text, CMenuItem* prev = 0);


        /**
         *  Setups the specified array of pages with relative page relationship
         *
         *  What this does is changes any target page (or previous page) on the pages of the array to be relative to the first entry
         *  So if the second page on the array points (by target page) to a page number '4' it will be set to point to the page of index 4 on the array
         *  This basically does (pages[i].SomePageReference += GetPageIndex(pages[0])) on all pages of the array
         *
         *  If any page reference is negative, it will be turned to positive, absolute referencing a page.
         *
         *  @param pages    The array of pages to setup
         *  @param npage    The number of pages on the array
         */
        void RelativeSetup(CMenuItem* pages, size_t npages);


        /*
         *  Gets an entry at the specified menu page
         *  @param page         The page to search the entry for
         *  @param entryname    The entry name to search for
         *  @return             Returns the entry found or null on failure or if page is null
         */
        static CMenuEntryData* GetEntry(CMenuItem* page, const char* entryname);

        /*
         *  Adds a new menu entry into the specified menu page
         *  @param page         The page to add the entry in
         *  @return             Returns the added entry or null on failure or if page is null
         */
        CMenuEntryData* AddEntry(CMenuItem* page);

        /*
         *  Adds a new menu entry into the specified menu page
         *  Be careful with this function, it will invalidate any index/pointer to the 'before' entry and the entries following it
         *  @param page         The page to add the entry in
         *  @param before       Add the entry before this one
         *  @return             Returns the added entry or null on failure or if page or before are null
         */
        CMenuEntryData* AddEntryBefore(CMenuItem* page, CMenuEntryData* before);



        /**
         *  Gets the pointer to the main menu manager
         *  @return             The main menu manager
         */
        static CMenuManager* GetMenuManager();

        /**
         *  Gets the currently displayed page index in the menu manager
         *  @param menumgr      The menu manager to look into
         *  @return             The currently displayed page index
         */
        static int GetPageFromMenuManager(CMenuManager* menumgr);
       
        /**
         *  Gets the currently selected entry index in the menu manager
         *  @param menumgr      The menu manager to look into
         *  @return             The currently selected entry index
         */
        static int GetEntryFromMenuManager(CMenuManager* menumgr);


    public:

        struct ActionInfo;
        struct UserInputInfo;
        struct BackgroundInfo; 
        struct DrawInfo;
        struct StateTextInfo;

        /**
            Events registering

            Things you should know:
                [*] You'll receive events for every entry and menu, you may filter only the events you care about (related to your menu or something)
                [*] The callbacks returns a boolean specifying if it should forward the event (to the game itself or another plugin)
                [*] The event information objects you receive are mutable and can modify the behaviour of the menu
                [*] You can register only one handler! (Of course this limitation is per ASI not global)
        */
        
        /**
         *  Registers an action event, this is triggered when an menu entry gets pressed.
         *  Check out ActionInfo for more details
         */
        void RegisterActionListener(std::function<bool(ActionInfo&)> handler);

        /**
         *  Registers an user input event, this is triggered every frame on the input processing
         *  Check out UserInputInfo for more details
         */
        void RegisterUserInputListener(std::function<bool(UserInputInfo&)> handler);

        /**
         *  Registers an background event, this is triggered every frame on the menu rendering
         *  Check out BackgroundInfo for more details
         */
        void RegisterBackgroundListener(std::function<bool(BackgroundInfo&)> handler);

        /**
         *  Registers an drawing event, this is triggered every frame on the menu rendering
         *  Check out DrawInfo for more details
         */
        void RegisterDrawListener(std::function<bool(DrawInfo&)> handler);

        /**
         *  Registers an state text event, this is triggered every frame on the menu rendering if a custom page is being displayed
         *  An state text event is those "ON/OFF", "1,2,3,...." texts after a menu entry
         *  Check out StateTextInfo for more details
         */
        void RegisterStateTextListener(std::function<bool(StateTextInfo&)> handler);




        /******** AH, TOO LAZY TO DOXYGEN THOSE STRUCTURES ********/

        // Base for any event information
        struct EventInfo
        {
            CMenuManager*         menumgr;      // The menu manager that triggered the event
            CMenuItem*            page;         // The menu page that triggered the event or null if none
            CMenuEntryData*       entry;        // The menu entry index in the page that triggered the event or null

            EventInfo(CMenuManager* menumgr, bool use_page, bool use_entry) :
                menumgr(menumgr),
                page(nullptr), entry(nullptr)
            {
                if(use_page)
                {
                    auto i = GetPageFromMenuManager(menumgr);
                    if(i >= 0 && i < max_pages)
                    {
                        if(page = Instance()->GetPage(i))
                        {
                            if(use_entry)
                            {
                                i = GetEntryFromMenuManager(menumgr);
                                if(i >= 0 && i < max_entries) entry = &page->m_aEntries[i];
                            }
                        }
                    }
                }
            }
        };

        // Base for any action event (triggered by clicking an menu entry)
        struct ActionInfo : EventInfo
        {
            unsigned char         action;       // The action triggered
            signed   char         wheel;        // The left/right movement (( < 0 is left; > 0 is right ))
            unsigned char         enter;        // The enter press

            ActionInfo(int wheel, int enter) :
                EventInfo(GetMenuManager(), true, true),
                action(entry? entry->m_nActionType : 0), wheel(wheel), enter(enter)
            {}
        };

        // Base for any user input event (triggered every frame)
        struct UserInputInfo : EventInfo
        {
            unsigned char down, up, enter, exit;    // Booleans, all obvious....
            char wheel;                             // The left/right movement (( < 0 is left; > 0 is right ))

            UserInputInfo(unsigned char down, unsigned char up, unsigned char enter, unsigned char exit, char wheel) :
                EventInfo(GetMenuManager(), true, true), down(down), up(up), enter(enter), exit(exit), wheel(wheel)
            {}
        };

        // Base for any background event (triggered every frame)
        struct BackgroundInfo : EventInfo
        {
            CSprite2d* sprite;                  // The sprite used for background or null for none
            CRect*     rect;                    // The rect to draw the sprite in, modify the value pointed by
            CRGBA*     rgba;                    // The color to draw the sprite in, modify the value pointed by

            BackgroundInfo(CSprite2d* sprite, CRect* rect, CRGBA* rgba) :
                EventInfo(GetMenuManager(), true, false), sprite(sprite), rect(rect), rgba(rgba)
            {}
        };

        // Base for any drawing event (triggered every frame)
        struct DrawInfo : EventInfo
        {
            unsigned char drawtitle;            // Boolean to specify if the menu title should be drawn

            DrawInfo(unsigned char drawtitle) :
                EventInfo(GetMenuManager(), true, true), drawtitle(drawtitle)
            {}
        };

        // Base for an state text event (triggered every frame if in a custom menu page)
        struct StateTextInfo : EventInfo
        {
            CText* ctext;                   // CText
            const char* gxtentry;           // GXT entry for the state
            const char* text;               // Raw text for the state  (may be null to use gxt entry)
            CMenuEntryData* parsing_entry;  // The entry being parsed to draw the state text

            StateTextInfo(CText* ctext, const char* gxtentry) :
                EventInfo(GetMenuManager(), true, true), ctext(ctext), gxtentry(gxtentry), text(nullptr)
            {}
        };






    /*
        Action Handler Patching
    */
    private:
        static std::function<bool(ActionInfo&)>& action_handler()
        { static std::function<bool(ActionInfo&)> x; return x; }

        typedef bool(__fastcall *action_handler_t)(void*, int, signed char, unsigned char);
        static action_handler_t& action_handler_ptr() { static action_handler_t x; return x; }

        static void PatchActionHandler();

        static bool __fastcall ActionHandler(void* self, int, signed char wheel, unsigned char enter);


    /*
        Background Handler Patching
    */
    private:
        static std::function<bool(BackgroundInfo&)>& background_handler()
        { static std::function<bool(BackgroundInfo&)> x; return x; }

        typedef void(__fastcall *background_handler_t)(CSprite2d*, int, CRect*, CRGBA*);
        static background_handler_t& background_handler_ptr() { static background_handler_t x; return x; }

        static void PatchBackgroundHandler();

        static void __fastcall BackgroundHandler(CSprite2d* self, int, CRect* rect, CRGBA* rgba);

    /*
        User Input Handler Patching
    */
    private:
        static std::function<bool(UserInputInfo&)>& userinput_handler()
        { static std::function<bool(UserInputInfo&)> x; return x; }

        typedef void(__fastcall *userinput_handler_t)(void*, int, unsigned char, unsigned char, unsigned char, unsigned char, char);
        static userinput_handler_t& userinput_handler_ptr() { static userinput_handler_t x; return x; }


        static void PatchUserInputHandler();

        static void __fastcall UserInputHandler(void* self, int, unsigned char down, unsigned char up, unsigned char enter, unsigned char exit, char wheel);


    /*
        Draw Handler Patching
    */
    private:
        static std::function<bool(DrawInfo&)>& draw_handler()
        { static std::function<bool(DrawInfo&)> x; return x; }

        typedef void(__fastcall *draw_handler_t)(void*, int, unsigned char);
        static draw_handler_t& draw_handler_ptr() { static draw_handler_t x; return x; }

        static void PatchDrawHandler();

        static void __fastcall DrawHandler(void* self, int, unsigned char drawtitle);
  
    /*
        State Text Patching
    */
        static std::function<bool(StateTextInfo&)>& state_text_handler()
        { static std::function<bool(StateTextInfo&)> x; return x; }

        typedef const char*(__fastcall *state_text_handler_t)(CText*, int, const char*);
        static state_text_handler_t& state_text_handler_ptr() { static state_text_handler_t x; return x; }

        static void PatchStateTextHandler();

        static const char* __fastcall StateTextHandler(CText* self, int, const char* gxtentry);


    private:
        void Patch(CMenuItem* entries);
        static int Register(short& used, short quantity, short max);
        void Initialise(MenuExtenderData* block);

    protected:
        static MenuExtender*& Instance()
        {
            static MenuExtender* inst;
            return inst;
        }
};



};
