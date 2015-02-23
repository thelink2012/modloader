/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <cstdint>
#include <functional>
#include "MenuExtender/MenuExtender.h"
#include <injector/calling.hpp>
#include <chrono>

using plugin::MenuExtender;
using std::placeholders::_1;

static const auto MENU_ACTION_DUMMY = MenuExtender::dummy_action;
static const auto MENU_ENTRY_DUMMY  = MenuExtender::dummy_type;

/*
 *  AbstractFrontendBase  
 *      Basic functionality derived from MenuExtender
 */
class AbstractFrontendBase : public MenuExtender
{
    public: // Basic Types

        using chrono_clock = std::chrono::steady_clock; // Clock used for measuring time passed
        using ms_t         = std::chrono::milliseconds; // Unit of time used for measuring

        // Measures time passed until a certain event can happen
        struct delayed_message
        {
            public:
                delayed_message() = default;
                delayed_message(ms_t delay) : time(now() + delay) {}
                
                // Time from the chrono_clock that the event can be completed
                ms_t completion_time()  { return time; }
                // Checks if the event has been completed
                bool has_completed()    { return check_completion(); }
                // Forces the event completion
                void complete_it()      { completed = true; }

            private:
                ms_t time;                      // Time of completion of the event
                bool completed  = false;        // Has the event been completed?

                // Gets the current moment in time from the chrono_clock
                ms_t now()
                {
                    return std::chrono::duration_cast<std::chrono::milliseconds>(chrono_clock::now().time_since_epoch());
                }

                // Checks if the time for the event has passed
                bool check_completion()
                {
                    if(!completed)
                        this->completed = (this->now() >= this->completion_time());
                    return completed;
                }
        };

        
    private:
        /*
         *  MenuEventListener
         *      Listens for the events of type EventType and passes it forward to it's callbacks
         */
        template<class EventType>
        class MenuEventListener
        {
            private:
                using func_t = std::function<bool(EventType&)>;
                using container_type = std::list<std::pair<CMenuItem*, func_t>>;
                using handle_type = void*;
                using delayed_storer = std::vector<std::pair<handle_type, delayed_message>>;

            private:
                chrono_clock clock;
                MenuExtender& menu;
                container_type handlers;
                delayed_storer delayed;
                void(MenuExtender::*RegisterHandler)(func_t);

            public:

                // Register this listener on the MenuExtender
                MenuEventListener(MenuExtender& menu, void(MenuExtender::*RegisterHandler)(func_t))
                    : menu(menu), RegisterHandler(RegisterHandler)
                {
                    (menu.*RegisterHandler)(GetBind());
                }

                // Unregisters this listener from the MenuExtender
                ~MenuEventListener()
                {
                    (menu.*RegisterHandler)(nullptr);
                }


                // Adds a event which is triggered only in the specified page
                handle_type AddEvent(CMenuItem* page, func_t handler)
                {
                    handlers.emplace_back(page, handler);
                    return HandleFromPair(handlers.back());
                }

                // Adds a event which is triggered only in the specified page...
                // This event should have priority over the ones registered by AddEvent
                handle_type AddEventFirstPriority(CMenuItem* page, func_t handler)
                {
                    handlers.emplace_front(page, handler);
                    return HandleFromPair(handlers.front());
                }

                // Removes a previosly registered event
                bool RemoveEvent(handle_type handle)
                {
                    return RemoveEventIt(handle) != handlers.end();
                }

                // Removes a previosly registered event after the specified miliseconds delay
                void RemoveEventDelayed(handle_type h, uint32_t delay = 0)
                {
                    delayed.emplace_back(h, delay);
                }

            protected:
                void RunDelayedEvents() // Remove events from RemoveEventDelayed
                {
                    for(auto it = delayed.begin(); it != delayed.end(); )
                    {
                        if(it->second.has_completed())
                        {
                            this->RemoveEvent(it->first);
                            it = delayed.erase(it);
                        }
                        else ++it;
                    }
                }

                bool HandlerFunc(EventType& info)
                {
                    bool result = true;
                    RunDelayedEvents();
                    {
                        for(auto& pair : handlers)
                        {
                            if(pair.first == nullptr || pair.first == info.page)
                            {
                                info.user = HandleFromPair(pair);
                                if(!pair.second(info))
                                {
                                    result = false;
                                    break;
                                }
                            }
                        }
                    }
                    //RunDelayedEvents();
                    return result;
                }


                typename container_type::iterator RemoveEventIt(handle_type handle)
                {
                    for(auto it = handlers.begin(); it != handlers.end(); ++it)
                    {
                        if(handle == HandleFromPair(*it))
                            return handlers.erase(it);
                    }
                    return handlers.end();
                }

                void* HandleFromPair(const std::pair<CMenuItem*, func_t>& pair)
                {
                    return (void*) &pair;
                }

                func_t GetBind()
                {
                    return std::bind(&MenuEventListener::HandlerFunc, this, _1);
                }
        };

        /*
         *  MenuEventMgr
         *      Manages the event for the specified MenuExtender callback and it's EventType
         *      It's a derivation of MenuEventListener
         */
        template<class EventType, void(MenuExtender::*RegisterHandlerFun)(std::function<bool(EventType&)>)>
        class MenuEventMgr : public MenuEventListener<EventType>
        {
            public:
                MenuEventMgr(MenuExtender& menu) : MenuEventListener<EventType>(menu, RegisterHandlerFun)
                {}
        };



    private:
        short* pPrevKeys;           // Previous key states
        short* pCurrKeys;           // Current key states
        short* pPrevBackspaceKey;   // Previous backspace state (not in the PrevKeys array)
        short* pCurrBackspaceKey;   // Current backspace state  (...)

    public:
        using TextEntry          = std::pair<std::string, bool>;    // .first = text or gxt key; .second = is_gxt
        using BackgroundEventMgr = MenuEventMgr<BackgroundInfo, &MenuExtender::RegisterBackgroundListener>;     // Menu background rendering
        using ActionEventMgr     = MenuEventMgr<ActionInfo, &MenuExtender::RegisterActionListener>;             // Menu actions handling
        using StateTextMgr       = MenuEventMgr<StateTextInfo, &MenuExtender::RegisterStateTextListener>;       // Menu entries state setup
        using UserInputMgr       = MenuEventMgr<UserInputInfo, &MenuExtender::RegisterUserInputListener>;       // User input handling
        using DrawEventMgr       = MenuEventMgr<DrawInfo, &MenuExtender::RegisterDrawListener>;                 // Menu drawing handling

        //
        AbstractFrontendBase()
        {
            auto pPrev = raw_ptr(injector::memory_pointer(0xB72F20).get());
            auto pCurr = raw_ptr(injector::memory_pointer(0xB73190).get());
            pPrevKeys = raw_ptr(pPrev + 0x18).get();
            pCurrKeys = raw_ptr(pCurr + 0x18).get();
            pPrevBackspaceKey = raw_ptr(pPrev + 0x254).get();
            pCurrBackspaceKey = raw_ptr(pCurr + 0x254).get();
            MenuExtender::Initialise();
        }

        // Checks if the user just pressed the specified keyboard 'key' (VKey)
        bool HasKeyJustPressed(int key)
        {
            return key >= 0 && key <= 127 && pCurrKeys[key] && !pPrevKeys[key];
        }

        // Checks if the user just pressed the BACKSPACE keyboard key
        bool HasJustPressedBackspace()
        {
            return *pCurrBackspaceKey && !*pPrevBackspaceKey;
        }

        // Returns the specified GXT/FXT label entry
        static TextEntry TextLabel(const std::string& label)
        {
            return TextEntry(label, true);
        }

        // Retruns a text entry which contains ON or OFF based on the boolean 'state'
        static TextEntry BoolLabel(bool state)
        {
            return TextLabel(state? "ML_ONN" : "ML_OFF");
        }

        // Returns a text entry which contains the number 'i' to be used in a menu state
        template<class T>
        static TextEntry NumberEntry(T i)
        {
            return TextEntry(std::to_string(i), false);
        }
};



class AbstractFrontend : public AbstractFrontendBase
{
    protected:
        // Event Managers
        BackgroundEventMgr evBackground;
        ActionEventMgr     evActions;
        StateTextMgr       evStateText;
        UserInputMgr       evUserInput;
        DrawEventMgr       evDraw;

        void* mCurrentHelperEvent = nullptr;                    // Helper handle
        CMenuItem* mCurrentHelperPage = nullptr;                // Page owning the currently displaying helper text
        std::pair<TextEntry, delayed_message> mCurrentHelper;   // Current helper text and timing

        static AbstractFrontend* Instance()
        { return (AbstractFrontend*) AbstractFrontendBase::Instance(); }

    public:
        struct MenuPage;
        struct MenuEntry;

        AbstractFrontend()
            : evBackground(*this), evActions(*this), evStateText(*this), evUserInput(*this), evDraw(*this)
        {
        }

        // Reports an game audio event
        void ReportAudioEvent(int id, float volumeChange = 0, float speed = 1)
        {
            //  CReference* CAudioEngine::ReportFrontendAudioEvent(CAudioEngine *this, int eventId, float volumeChange, float speed)
            injector::thiscall<void(void*, int, float, float)>::call<0x506EA0>(
                                                    (void*) injector::lazy_pointer<0xB6BC90>().get(),
                                                    id, volumeChange, speed);
        }
        // Outputs to the audio device a error sound
        void ReportError()
        {
            ReportAudioEvent(4);
        }

        // Reports an error by helper text followed by an error audio
        void ReportError(const TextEntry& entry)
        {
            this->SetHelperText(entry, nullptr, 2000);
            this->ReportError();
        }

        // Outputs to the audio device a highlight sound
        void ReportHighlight()
        {
            ReportAudioEvent(3);
        }
        
        // Outputs to the audio device a state change sound
        void ReportStateChange()
        {
            ReportAudioEvent(1);
        }

        void DrawHelperText(const TextEntry& entry)
        {
            if(entry.second)
            {
                // CMenuManager::DisplayHelperText(CMenuManager* this, const char* gxtentry)
                injector::thiscall<void(CMenuManager*, const char*)>::call<0x57E240>(GetMenuManager(), entry.first.data());
            }
        }

        void DrawMessageScreen(const TextEntry& entry)
        {
            if(entry.second)
            {
                // CMenuManager::MessageScreen(CMenuManager* this, const char* gxtentry, char bFillScreenBlack, char bFillOnce)
                injector::thiscall<void(CMenuManager*, const char*, char, char)>::call<0x579330>(GetMenuManager(), entry.first.data(), 0, 0);
            }
        }


        // Sets the current helper text into the text entry 'entry' with the duration in miliseconds of 'delay_ms'
        // If 'for_page' is not nullptr the helper text will disappear when the user is not on the specified page.
        void SetHelperText(const TextEntry& entry, CMenuItem* for_page = nullptr, uint32_t delay_ms = 5000)
        {
            this->mCurrentHelper.first  = entry;
            this->mCurrentHelper.second = delayed_message(ms_t(delay_ms));
            this->mCurrentHelperPage    = for_page? for_page : GetPage(GetCurrentPageIndex());

            if(!mCurrentHelperEvent)    // if not holding any helper handle take one (do this only once in ever)
            {
                mCurrentHelperEvent = OnDraw(nullptr, [this](DrawInfo& info)
                {
                    if(mCurrentHelper.first.first.size())   // has any helper text
                    {
                        if(info.page != mCurrentHelperPage) ClearHelperText();
                        else if(!mCurrentHelper.second.has_completed()) DrawHelperText(mCurrentHelper.first);
                        else ClearHelperText();
                    }
                    return true;
                });
            }
        }

        // Clears the currently displaying helper text
        void ClearHelperText()
        {
            mCurrentHelper.first.first.clear();
        }

        // Setups the background for the specified page to be the specified sprite index, drawed in the specified 'rect' area and colored with 'color'
        // NOTE: The rect IS NOT in local space
        void* SetupBackground(CMenuItem* page, int index, CRect rect, CRGBA color)
        {
            return OnBackground(page, [=](BackgroundInfo& info)
            {
                info.sprite = (CSprite2d*)((uintptr_t)(GetMenuManager()) + 0x12C + 4*index);
                *info.rect = rect;
                *info.rgba = color;
                return true;
            });
        }

        // Fetches the user input from the current frame
        std::string FetchInputText()
        {
            std::string text;
            for(auto i = 0; i < 256; ++i)
                if(HasKeyJustPressed(i)) text.push_back(i);
            return text;
        }


        // Setups a handler to be called whenever any kind of action happens in a page
        void* OnAction(CMenuItem* page, std::function<bool(ActionInfo&)> handler)
        {
            return evActions.AddEvent(page, [=](ActionInfo& info)
            {
                if(!handler(info)) return CannotStopPropagation(info.action);
                return true;
            });
        }

        // Setups a handler to be called whenever any kind of action happens to the specified entry
        void* OnAction(CMenuItem* page, CMenuEntryData* entry, std::function<bool(ActionInfo&)> handler)
        {
            return OnAction(page, [=](ActionInfo& info)
            {
                return info.entry == entry? handler(info) : true;
            });
        }

        // Setups a handler to take care of user input (enter/exit/left/right) in a menu page
        void* OnUserInput(CMenuItem* page, std::function<bool(UserInputInfo&)> handler)
        {
            return evUserInput.AddEvent(page, handler);
        }

        // Setups a handler to do custom rendering in a menu page
        void* OnDraw(CMenuItem* page, std::function<bool(DrawInfo&)> handler)
        {
            return evDraw.AddEvent(page, handler);
        }

        // Adds a manual background handling event
        void* OnBackground(CMenuItem* page, std::function<bool(BackgroundInfo&)> handler)
        {
            return evBackground.AddEvent(page, handler);
        }

        // Setups a handler to be called every frame in a menu page
        void* OnTick(CMenuItem* page, std::function<void()> handler)
        {
            return evDraw.AddEvent(page, [handler](DrawInfo&)
            {
                handler();
                return true;
            });
        }

        // Setups a state text drawer for the specified menu entry
        // The handler receives a reference to a TextEntry which should be set up with the text entry to be drawn for the state of the entry
        void* OnStateText(CMenuItem* page, CMenuEntryData* entry, std::function<bool(TextEntry&)> handler)
        {
            return evStateText.AddEvent(page, [=](StateTextInfo& info)
            {
                if(info.parsing_entry == entry)
                {
                    static TextEntry pair;
                    pair.first.clear();
                    pair.second = true;

                    if(handler(pair))
                    {
                        if(pair.second) // is_gxt?
                            info.gxtentry = pair.first.data();
                        else
                            info.text = pair.first.data();
                    }
                }
                return true;
            });
        }








        
        /*
         *  MenuEntry
         *      Represents one of many menu entries in a page
         */
        struct MenuEntry
        {
            public:
                MenuPage*           pOwner;         // The page which owns this entry
                CMenuEntryData*     pEntry;         // The non-abstract entry

            private:
                // Event abstraction
                TextEntry           mHelper;        // The helper text which displays when you go over this entry
                void*               hHelperEvent;   // The handler of the OnDraw event for the helper
                TextEntry           mState;         // The state of this entry (if any)
                void*               hStateEvent;    // The OnStateText handle for mState
                void*               hStatefulEvent; //

                //
                std::function<void(MenuEntry&)> mStatefulChange;

            public:

                MenuEntry() :
                    hHelperEvent(nullptr), hStateEvent(nullptr), hStatefulEvent(nullptr)
                {}

                ~MenuEntry()
                {
                    if(hHelperEvent) AbstractFrontend::Instance()->evDraw.RemoveEvent(hHelperEvent);
                    if(hStateEvent)  AbstractFrontend::Instance()->evStateText.RemoveEvent(hStateEvent);
                    if(hStatefulEvent) AbstractFrontend::Instance()->evActions.RemoveEvent(hStatefulEvent); 
                }

                // Initialises the entry from a existing page and abstract entry
                void Initialise(MenuPage* page, CMenuEntryData* entry)
                {
                    this->pOwner = page;
                    this->pEntry = entry;
                }

            
                // Sets the helper text for this entry
                void SetHelper(TextEntry helper);

                // Sets the state text for this entry
                void SetState(TextEntry state);

                // Sets the handler for when the user changes a stateful entry
                // NOTE: This is still valid when you've called SetupStatefulEntry or so
                void OnStateChange(std::function<void(MenuEntry&)> cb)
                {
                    this->mStatefulChange = std::move(cb);
                }

                // Setups a stateful entry which contains either ON/OFF state
                std::function<bool(ActionInfo&)> SetupBooleanEntry(std::reference_wrapper<bool> value)
                {
                   return SetupStatefulEntry(value, std::function<TextEntry(bool)>(BoolLabel), [](bool value, ActionInfo&)
                   {
                       return !value;
                   });
                }

                // Setups a stateful entry which contains a number in the range min-max, changing the entry by the specified amount of steps
                template<class T>
                std::function<bool(ActionInfo&)> SetupIntegerEntry(std::reference_wrapper<T> value, T min, T max, T step)
                {
                    return SetupStatefulEntry(value, std::ref(NumberEntry<T>), [min, max, step](const T& value, ActionInfo& info)
                    {
                        return std::min(std::max((value + step * info.wheel), min), max);
                    });
                }

                // Can decay into the non-abstract entry pointer
                operator CMenuEntryData*()
                {
                    return pEntry;
                }

                // Setups this entry in the menu to be stateful (i.e. have a state)
                // The specified value reference is the object state we'll relate to, 'to_text' is the functor which converts such object to a text format
                // and 'operation' is the operation that takes place when a user input goes tho this entry
                template<class T, class ToText, class Operation>
                std::function<bool(ActionInfo&)> SetupStatefulEntry(std::reference_wrapper<T> value, ToText to_text, Operation operation)
                {
                    // Helper to update the state of 
                    auto UpdateState = [this, to_text, value]()
                    {
                        this->SetState(to_text(value));
                    };

                    if(!hStatefulEvent)
                    {
                        // Whenever a user actions goes tho this entry
                        this->hStatefulEvent = Instance()->OnAction(*this->pOwner, *this, [this, value, operation, UpdateState](ActionInfo& info)
                        {
                            value.get() = operation(value, info);
                            UpdateState();
                            Instance()->ReportStateChange();                // audio
                            if(mStatefulChange) mStatefulChange(*this);
                            return true;
                        });
                    }

                    // Returns a functor capable of updating the text state of the entry
                    return [UpdateState](ActionInfo& info)
                    {
                        UpdateState();
                        return true;
                    };
                }

        };
        
        /*
         *  MenuPage
         *      Represents a abstract menu page
         */
        struct MenuPage
        {
            private:
                CMenuItem* pPage     = nullptr;     // Non-abstract page
                MenuEntry mEntries[max_entries];    // Menu entries related to this page

            public:

                // Initialises the content of this abstract page with the content of a non-abstract page
                void Initialise(CMenuItem* pPage)
                {
                    this->pPage = pPage;
                    for(size_t i = 0; i < (size_t)max_entries; ++i)
                        mEntries[i].Initialise(this, &pPage->m_aEntries[i]);
                }

                // Finds the MenuEntry with the specified label in it
                MenuEntry* GetEntry(const char* label)
                {
                    if(auto entry = Instance()->GetEntry(pPage, label))
                        return &mEntries[entry - pPage->m_aEntries];
                    return nullptr;
                }
                
                // Finds the MenuEntry at the position 'i'
                MenuEntry* GetEntry(size_t i)
                {
                    return &mEntries[i];
                }

                // Can decay into the non-abstract page pointer
                operator CMenuItem*()
                {
                    return pPage;
                }
        };




    private:
        // The event should be always propagated when the action is the dummy one
        static bool CannotStopPropagation(int action)
        {
            return action == MENU_ACTION_DUMMY;
        }

};



void AbstractFrontend::MenuEntry::SetState(TextEntry state)
{
    this->mState = std::move(state);
    if(!this->hStateEvent)
    {
        hStateEvent = AbstractFrontend::Instance()->OnStateText(*pOwner, pEntry, [this](TextEntry& text)
        {
            text = this->mState;
            return true;
        });
    }
}

void AbstractFrontend::MenuEntry::SetHelper(TextEntry helper)
{
    if(helper.first.size())
    {
        this->mHelper = std::move(helper);
        if(!this->hHelperEvent)
        {
            hHelperEvent = AbstractFrontend::Instance()->OnDraw(*pOwner, [this](DrawInfo& info)
            {
                if(info.entry == this->pEntry)
                {
                    Instance()->ClearHelperText();
                    Instance()->DrawHelperText(mHelper);
                }
                return true;
            });
        }
    }
}