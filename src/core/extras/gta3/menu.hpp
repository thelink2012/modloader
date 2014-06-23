/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#pragma once
#include <cstdint>
#include <functional>
#include "MenuExtender/MenuExtender.h"

// TODO THIS HEADER NEEDS REFACTORING

using plugin::MenuExtender;
using std::placeholders::_1;

static const auto MENU_ACTION_DUMMY = MenuExtender::dummy_action;
static const auto MENU_ENTRY_DUMMY  = MenuExtender::dummy_type;


class AbstractFrontendBase : public MenuExtender
{
    protected:
        template<class EventType>
        class MenuEventListener
        {
            private:
                typedef std::function<bool(EventType&)> func_t;
                std::vector<std::pair<CMenuItem*, func_t>> handlers;

                MenuExtender& menu;
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


                void AddEvent(CMenuItem* page, func_t handler)
                {
                    handlers.push_back(std::make_pair(page, handler));
                }

            protected:
                bool HandlerFunc(EventType& info)
                {
                    for(auto& pair : handlers)
                    {
                        if(info.page == pair.first)
                        {
                            if(!pair.second(info))
                                return false;
                        }
                    }
                    return true;
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

        AbstractFrontendBase()
        {
            MenuExtender::Initialise();
        }

        static TextEntry TextLabel(const std::string& label)
        {
            return TextEntry(label, true);
        }

        static TextEntry BoolLabel(bool state)
        {
            return TextLabel(state? "_ML_ONN" : "_ML_OFF");
        }

        template<class T>
        static TextEntry NumberLabel(T i)
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

        static AbstractFrontend* Instance() { return (AbstractFrontend*) AbstractFrontendBase::Instance(); }

        AbstractFrontend()
            : evBackground(*this), evActions(*this), evStateText(*this), evUserInput(*this), evDraw(*this)
        {
        }

        static bool CannotStopPropagation(int action)
        {
            return action == MENU_ACTION_DUMMY;
        }

        void SetBackground(CMenuItem* page, int index, CRect rect, CRGBA color)
        {
            evBackground.AddEvent(page, [=](BackgroundInfo& info)
            {
                info.sprite = (CSprite2d*)((uintptr_t)(GetMenuManager()) + 0x12C + 4*index);
                *info.rect = rect;
                *info.rgba = color;
                return true;
            });
        }

        // pair<text, is_gxt = true>
        void OnStateText(CMenuItem* page, CMenuEntryData* entry, std::function<bool(std::pair<std::string, bool>&)> handler)
        {
            evStateText.AddEvent(page, [=](StateTextInfo& info)
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



        void OnUserInput(CMenuItem* page, std::function<bool(UserInputInfo&)> handler)
        {
            evUserInput.AddEvent(page, handler);
        }

        void OnDraw(CMenuItem* page, std::function<bool(DrawInfo&)> handler)
        {
            evDraw.AddEvent(page, handler);
        }

        void OnTick(CMenuItem* page, std::function<void()> handler)
        {
            evDraw.AddEvent(page, [handler](DrawInfo&)
            {
                handler();
                return true;
            });
        }




        void OnAction(CMenuItem* page, std::function<bool(ActionInfo&)> handler)
        {
            evActions.AddEvent(page, [=](ActionInfo& info)
            {
                if(!handler(info)) return CannotStopPropagation(info.action);
                return true;
            });
        }

        void OnAction(CMenuItem* page, int action, std::function<bool(ActionInfo&)> handler)
        {
            OnAction(page, [=](ActionInfo& info)
            {
                return info.action == action? handler(info) : true;
            });
        }

        void OnAction(CMenuItem* page, CMenuEntryData* entry, std::function<bool(ActionInfo&)> handler)
        {
            OnAction(page, [=](ActionInfo& info)
            {
                return info.entry == entry? handler(info) : true;
            });
        }






        struct MenuPage;

        struct MenuEntry
        {
            MenuPage*           pOwner;
            CMenuEntryData*     pEntry;

            TextEntry           state;
            bool                bHasState = false;

            std::function<void(MenuEntry&)> mStatefulChange;

            void Initialise(MenuPage* page, CMenuEntryData* entry)
            {
                this->pOwner = page;
                this->pEntry = entry;
            }

            void SetState(const TextEntry& state);

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



        void AbstractFrontend::MenuEntry::SetState(const TextEntry& state)
        {
                this->state = state;
                if(!bHasState)
                {
                    bHasState = true;
                    AbstractFrontend::Instance()->OnStateText(*pOwner, pEntry, [this](TextEntry& text)
                    {
                        text = this->state;
                        return true;
                    });
                }
        }