/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <cstdint>
#include <functional>
#include "MenuExtender/MenuExtender.h"
#include <injector/calling.hpp>
#include <chrono>

// TODO THIS HEADER NEEDS REFACTORING

using plugin::MenuExtender;
using std::placeholders::_1;

static const auto MENU_ACTION_DUMMY = MenuExtender::dummy_action;
static const auto MENU_ENTRY_DUMMY  = MenuExtender::dummy_type;


class AbstractFrontendBase : public MenuExtender
{
    protected:
        using chrono_clock = std::chrono::steady_clock;
        using ms_t         = std::chrono::milliseconds;

        template<class T>
        static inline ms_t to_ms(const T& d)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(d);
        }

        struct delayed_message
        {
            private:
                ms_t time;
                bool completed  = false;

                ms_t now()          { return to_ms(chrono_clock::now().time_since_epoch()); }
                bool check_completion()
                {
                    if(!completed) this->completed = (now() >= completion_time());
                    return completed;
                }

            public:
                delayed_message() = default;
                delayed_message(ms_t delay) : time(now() + delay) {}
                
                ms_t completion_time()  { return time; }
                bool has_completed()    { return check_completion(); }
                void complete_it()      { completed = true; }

        };


        template<class EventType>
        class MenuEventListener
        {
            private:
                typedef std::function<bool(EventType&)> func_t;
                typedef std::list<std::pair<CMenuItem*, func_t>> container_type;
                typedef void*                                    handle_type;
                typedef std::vector<std::pair<handle_type, delayed_message>>  delayed_storer;

                chrono_clock clock;
                MenuExtender& menu;
                container_type handlers;
                delayed_storer delayed;
                void(MenuExtender::*RegisterHandler)(func_t);

            public:
                MenuEventListener(MenuExtender& menu, void(MenuExtender::*RegisterHandler)(func_t))
                    : menu(menu), RegisterHandler(RegisterHandler)
                {
                    (menu.*RegisterHandler)(GetBind());
                }

                ~MenuEventListener()
                {
                    (menu.*RegisterHandler)(nullptr);
                }


                handle_type AddEvent(CMenuItem* page, func_t handler)
                {
                    handlers.emplace_back(page, handler);
                    return HandleFromPair(handlers.back());
                }

                handle_type AddEventFirstPriority(CMenuItem* page, func_t handler)
                {
                    handlers.emplace_front(page, handler);
                    return HandleFromPair(handlers.front());
                }

                bool RemoveEvent(handle_type handle)
                {
                    return RemoveEventIt(handle) != handlers.end();
                }

                void RemoveEventDelayed(handle_type h, uint32_t delay = 0)
                {
                    delayed.emplace_back(h, delay);
                }

            protected:
                void RunDelayedEvents()
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

        template<class EventType, void(MenuExtender::*RegisterHandlerFun)(std::function<bool(EventType&)>)>
        class MenuEventMgr : public MenuEventListener<EventType>
        {
            public:
                MenuEventMgr(MenuExtender& menu) : MenuEventListener<EventType>(menu, RegisterHandlerFun)
                {}
        };

    public:
        using TextEntry          = std::pair<std::string, bool>;
        using BackgroundEventMgr = MenuEventMgr<BackgroundInfo, &MenuExtender::RegisterBackgroundListener>;
        using ActionEventMgr     = MenuEventMgr<ActionInfo, &MenuExtender::RegisterActionListener>;
        using StateTextMgr       = MenuEventMgr<StateTextInfo, &MenuExtender::RegisterStateTextListener>;
        using UserInputMgr       = MenuEventMgr<UserInputInfo, &MenuExtender::RegisterUserInputListener>;
        using DrawEventMgr       = MenuEventMgr<DrawInfo, &MenuExtender::RegisterDrawListener>;

        short* pPrevKeys;
        short* pCurrKeys;
        short* pPrevBackspaceKey;
        short* pCurrBackspaceKey;

        AbstractFrontendBase()
        {
            // XXX LOOK AT THIS PUT ON ADDR
            auto pPrev = raw_ptr(injector::memory_pointer(0xB72F20).get());
            auto pCurr = raw_ptr(injector::memory_pointer(0xB73190).get());
            pPrevKeys = raw_ptr(pPrev + 0x18).get();
            pCurrKeys = raw_ptr(pCurr + 0x18).get();
            pPrevBackspaceKey = raw_ptr(pPrev + 0x254).get();
            pCurrBackspaceKey = raw_ptr(pCurr + 0x254).get();
            MenuExtender::Initialise();
        }

        bool HasKeyJustPressed(int key)
        {
            return key >= 0 && key <= 127 && pCurrKeys[key] && !pPrevKeys[key];
        }

        bool HasJustPressedBackspace()
        {
            return *pCurrBackspaceKey && !*pPrevBackspaceKey;
        }

        static TextEntry TextLabel(const std::string& label)
        {
            return TextEntry(label, true);
        }

        static TextEntry BoolLabel(bool state)
        {
            return TextLabel(state? "ML_ONN" : "ML_OFF");
        }

        template<class T>
        static TextEntry NumberEntry(T i)
        {
            return TextEntry(std::to_string(i), false);
        }
};




class AbstractFrontend : public AbstractFrontendBase
{
    public:
        BackgroundEventMgr evBackground;
        ActionEventMgr     evActions;
        StateTextMgr       evStateText;
        UserInputMgr       evUserInput;
        DrawEventMgr       evDraw;

        int mCurrentHelperTry = 0;
        void* mCurrentHelperEvent = nullptr;
        CMenuItem* mCurrentHelperPage = nullptr;
        std::pair<TextEntry, delayed_message> mCurrentHelper;

        static AbstractFrontend* Instance() { return (AbstractFrontend*) AbstractFrontendBase::Instance(); }

        AbstractFrontend()
            : evBackground(*this), evActions(*this), evStateText(*this), evUserInput(*this), evDraw(*this)
        {
        }


        void ReportAudioEvent(int id, float volumeChange = 0, float speed = 1)
        {
            // XXX LOOK AT THIS PUT ON ADDR
            // 0x506EA0       CReference* CAudioEngine::ReportFrontendAudioEvent(CAudioEngine *this, int eventId, float volumeChange, float speed)
            injector::call<void(void*, int, float, float)>::thiscall(
                                                    0x506EA0,
                                                    injector::lazy_pointer<0xB6BC90>().get().get<void>(),
                                                    id, volumeChange, speed);
        }

        void ReportError()
        {
            ReportAudioEvent(4);
        }

        void ReportHighlight()
        {
            ReportAudioEvent(3);
        }
        
        void ReportStateChange()
        {
            ReportAudioEvent(1);
        }

        void ReportError(const TextEntry& entry)
        {
            this->SetHelperText(entry, nullptr, 2000);
            this->ReportError();
        }



        void DrawHelperText(const TextEntry& entry)
        {
            if(entry.second)
            {
                // XXX LOOK AT THIS PUT ON ADDR
                // 0x57E240       CMenuManager::DisplayHelperText(CMenuManager* this, const char* gxtentry)
                injector::call<void(CMenuManager*, const char*)>::thiscall(0x57E240, GetMenuManager(), entry.first.data());
            }
        }

        void DrawMessageScreen(const TextEntry& entry)
        {
            if(entry.second)
            {
                // XXX LOOK AT THIS PUT ON ADDR
                // 0x579330       int CMenuManager::MessageScreen(CMenuManager* this, const char* gxtentry, char bFillScreenBlack, char bFillOnce)
                injector::call<void(CMenuManager*, const char*, char, char)>::thiscall(0x579330, GetMenuManager(), entry.first.data(), 0, 0);
            }
        }


        void SetHelperText(const TextEntry& entry, CMenuItem* for_page = nullptr, uint32_t delay_ms = 5000)
        {
            this->mCurrentHelper.first  = entry;
            this->mCurrentHelper.second = delayed_message(ms_t(delay_ms));
            this->mCurrentHelperPage    = for_page? for_page : GetPage(GetCurrentPageIndex());

            if(!mCurrentHelperEvent)
            {
                mCurrentHelperEvent = OnDraw(nullptr, [this](DrawInfo& info)
                {
                    if(mCurrentHelper.first.first.size())
                    {
                        if(info.page != mCurrentHelperPage) ClearHelperText();
                        else if(!mCurrentHelper.second.has_completed()) DrawHelperText(mCurrentHelper.first);
                        else ClearHelperText();
                    }
                    return true;
                });
            }
        }

        void ClearHelperText()
        {
            mCurrentHelperTry = 0;
            mCurrentHelper.first.first.clear();
        }


        std::string FetchInputText()
        {
            std::string text;
            for(auto i = 0; i < 256; ++i)
                if(HasKeyJustPressed(i)) text.push_back(i);
            return text;
        }




        


        static bool CannotStopPropagation(int action)
        {
            return action == MENU_ACTION_DUMMY;
        }


        



        void* OnBackground(CMenuItem* page, std::function<bool(BackgroundInfo&)> handler)
        {
            return evBackground.AddEvent(page, handler);
        }

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

        // pair<text, is_gxt = true>
        void* OnStateText(CMenuItem* page, CMenuEntryData* entry, std::function<bool(std::pair<std::string, bool>&)> handler)
        {
            return evStateText.AddEvent(page, [=](StateTextInfo& info)
            {
                if(info.parsing_entry == entry)
                {
                    static std::pair<std::string, bool> pair;

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



        void* OnUserInput(CMenuItem* page, std::function<bool(UserInputInfo&)> handler)
        {
            return evUserInput.AddEvent(page, handler);
        }

        void* OnDraw(CMenuItem* page, std::function<bool(DrawInfo&)> handler)
        {
            return evDraw.AddEvent(page, handler);
        }

        void* OnTick(CMenuItem* page, std::function<void()> handler)
        {
            return evDraw.AddEvent(page, [handler](DrawInfo&)
            {
                handler();
                return true;
            });
        }




        void* OnAction(CMenuItem* page, std::function<bool(ActionInfo&)> handler)
        {
            return evActions.AddEvent(page, [=](ActionInfo& info)
            {
                if(!handler(info)) return CannotStopPropagation(info.action);
                return true;
            });
        }

        /*
        void* OnAction(CMenuItem* page, int action, std::function<bool(ActionInfo&)> handler)
        {
            return OnAction(page, [=](ActionInfo& info)
            {
                return info.action == action? handler(info) : true;
            });
        }
        */

        void* OnAction(CMenuItem* page, CMenuEntryData* entry, std::function<bool(ActionInfo&)> handler)
        {
            return OnAction(page, [=](ActionInfo& info)
            {
                return info.entry == entry? handler(info) : true;
            });
        }






        struct MenuPage;

        struct MenuEntry
        {
            MenuPage*           pOwner;
            CMenuEntryData*     pEntry;

            TextEntry           mHelper;
            void*               hHelperEvent = nullptr;

            TextEntry           mState;
            void*               hStateEvent = nullptr;

            std::function<void(MenuEntry&)> mStatefulChange;

            void Initialise(MenuPage* page, CMenuEntryData* entry)
            {
                this->pOwner = page;
                this->pEntry = entry;
            }

            void SetState(TextEntry state);
            void SetHelper(TextEntry helper);

            void OnStatefulChange(std::function<void(MenuEntry&)> cb)
            {
                this->mStatefulChange = std::move(cb);
            }

            template<class T, class ToText, class Operation>
            std::function<bool(ActionInfo&)> SetupStatefulEntry(std::reference_wrapper<T> value, ToText to_text, Operation operation)
            {
                auto UpdateState = [this, to_text, value]()
                {
                    this->SetState(to_text(value));
                };

                Instance()->OnAction(*this->pOwner, *this, [this, value, operation, UpdateState](ActionInfo& info)
                {
                    value.get() = operation(value, info);
                    UpdateState();
                    Instance()->ReportStateChange();
                    if(mStatefulChange) mStatefulChange(*this);
                    return true;
                });

                return [UpdateState](ActionInfo& info)
                {
                    UpdateState();
                    return true;
                };
            }

            std::function<bool(ActionInfo&)> SetupBooleanEntry(std::reference_wrapper<bool> value)
            {
               return SetupStatefulEntry(value, std::function<TextEntry(bool)>(BoolLabel), [](bool value, ActionInfo&)
               {
                   return !value;
               });
            }

            template<class T>
            std::function<bool(ActionInfo&)> SetupIntegerEntry(std::reference_wrapper<T> value, T min, T max, T step)
            {
                return SetupStatefulEntry(value, std::ref(NumberLabel<T>), [min, max, step](const T& value, ActionInfo& info)
                {
                    return std::min(std::max((value + step * info.wheel), min), max);
                });
            }


            operator CMenuEntryData*()
            {
                return pEntry;
            }
        };

        struct MenuPage
        {
            private:
                CMenuItem* pPage     = nullptr;
                MenuEntry mEntries[max_entries];

            public:
                void Initialise(CMenuItem* pPage)
                {
                    this->pPage = pPage;
                    for(int i = 0; i < max_entries; ++i)
                        mEntries[i].Initialise(this, &pPage->m_aEntries[i]);
                }

                MenuEntry* GetEntry(const char* label)
                {
                    if(auto entry = Instance()->GetEntry(pPage, label))
                        return &mEntries[entry - pPage->m_aEntries];
                    return nullptr;
                }
                
                MenuEntry* GetEntry(int i)
                {
                    return &mEntries[i];
                }

                MenuEntry* GetEntries()
                {
                    return mEntries;
                }

                void SetEntryState(const char* label, const TextEntry& state)
                {
                    if(auto entry = GetEntry(label))
                        entry->SetState(state);
                }

                operator CMenuItem*()
                {
                    return pPage;
                }
        };

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