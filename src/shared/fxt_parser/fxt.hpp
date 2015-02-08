/*
 *  Injectors - Fake GTA Text Implementation
 *	Header with helpful stuff for ASI memory hacking
 *
 *  (C) 2014 LINK/2012 <dma_2012@hotmail.com>
 * 
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#pragma once
#include <injector/hooking.hpp>
#include <map>

namespace injector
{
    //
    //  for a .fxt parser check out fxt_parser.hpp
    //
    
    /*
     *  Text hooking managing
     * 
     *      @TextMap: A map to store the key-value pair (normally a std::map), key should be a hash and value should be a std::string
     *      @UpperHashFunctor: Functor that returns the hash from a key
     *      @CText_Get: The address to the CText::Get function to hook into
     * 
     *      Usage example:
     *          basic_fxt_manager<std::map<uint32_t, std::string>, CUpperCaseHashingFunctor> manager;
     *          manager.patch();                // (OPTIONAL) Inject it into the game code
     *          manager.add("KEY", "VALUE");    // Add a new key-value pair into the game
     *                                          // The '.add' call already calls '.patch', so .patch is optional
     *
     *      Used Addresses:
     *          @CText_Get  - CText::Get routine
     *          0x748CFB    - When 'make_samp_compatible()' is called
     * 
     */
    template<class TextMap, class UpperHashFunctor, uintptr_t CText_Get = 0x6A0050>
    class basic_fxt_manager
    {
        public:
            typedef const char* (__fastcall *GetType)(void*, int, const char*);
            typedef typename TextMap::key_type hash_type;  
            typedef std::map<hash_type, TextMap> TableMap;

        private:
            struct static_data {
                bool is_enabled = false;            // Marks if this fxt manager is enabled or disabled (cannot unhook the game, so we need this)
                bool can_patch  = true;             // Can it patch the game automatically when add() or such is called?
                bool patched    = false;            // Has the patch happened?
                TableMap tmap;                      // Table of a map of strings
                memory_pointer_raw GetText;         // Store the raw pointer that CText::Get is located at
                memory_pointer_raw BefGet;          // The previous offset for CText::Get (before patching)
                memory_pointer_raw BefSamp;         // The previous offset for the SAMP compatibility hooked func (before patching)
                scoped_jmp         JmpHook;
            };

            static static_data& data()
            {
                static static_data data;
                return data;
            }

        public:

            /*
             *  Adds a GXT @key - @value pair to the text map for use in our GxtHook 
             */
            static void add(const char* key, const char* value, hash_type table = 0)
            {
                if(data().can_patch) patch();
                data().tmap[table][GetHash(key)] = value;
            }

            /*
             *  Overrides the specified GXT @key
             */
            static void set(const char* key, const char* value, hash_type table = 0)
            {
                return add(key, value, table);
            }

            /*
             *  Removes a previosly used table (i.e. keys added)
             */
            static void remove_table(hash_type table = 0)
            {
                data().tmap.erase(table);
            }

            /*
             *  Returns the value from @key in the CText object @ctext
             */
            static const char* get(void* ctext, const char* key)
            {
                return ((GetType) data().GetText.get())(ctext, 0, key);
            }

            /*
             *  Returns the value from @key in the current fxt container
             */
            static const char* get(const char* key, hash_type table = 0)
            {
                auto it_table = data().tmap.find(table);
                if(it_table != data().tmap.end())
                {
                    auto it_key = it_table->second.find(GetHash(key));
                    if(it_key != it_table->second.end())
                        return it_key->second.c_str();
                }
                return "";
            }


            

            /*
             *  Enable the FXT hook
             */
            static void enable(bool enable = true)
            {
                if(enable && data().can_patch) patch();
                data().is_enabled = enable;
            }

            /*
             *  Avoids automatic patching when add() or such gets called 
             */
            static void disable_patching(bool disable = true)
            {
                data().can_patch = !disable;
            }

            /*
             *  Makes this SAMP compatible
             */
            static void make_samp_compatible()
            {
                if(GetModuleHandleA("samp"))
                {
                    disable_patching();
                    data().BefSamp = MakeCALL(0x748CFB, raw_ptr(SampFixHook));
                }
            }





            /*
             *  Patches the game to work with our fxt table
             */
            static void patch()
            {
                if(data().patched == false)
                {
                    DWORD oldprotect;
                    data().patched = true;
                    enable();

                    data().GetText = memory_pointer(CText_Get).get<void>();         // Save the CText::Get pointer in raw form for fast access
                    UnprotectMemory(data().GetText, 5, oldprotect);    // unprotect this region forever
                    MakeHook();
                }
            }

            
        private:    // Internal stuff
            
            // Hooked CText::Get
            static const char* __fastcall GxtHook(void* self, int, const char* key)
            {
                if(data().is_enabled)
                {
                    if(const char* value = FindFromKey(key))
                        return value;
                }
                injector::scoped_basic<5> save_hook;
                save_hook.save(data().GetText.get(), 5, false);
                UnHook();
                auto result = ((GetType) data().GetText.get())(self, 0, key);
                MakeHook();
                return result;
            }

            // SAMP compatibility hook
            static void SampFixHook()
            {
                patch();
                return ((void (*)()) data().BefSamp.get())();
            }


            // Get hash from key
            static hash_type GetHash(const char* key)
            {
                UpperHashFunctor fun;
                return fun(key);
            }

            // Finds value from key
            static const char* FindFromKey(const char* key)
            {
                auto& tables = data().tmap;
                auto key_hash = GetHash(key);
                
                for(auto t = tables.begin(); t != tables.end(); ++t)
                {
                    auto it = t->second.find(key_hash);
                    if(it != t->second.end()) return it->second.c_str();
                }
                return nullptr;
            }

            static void MakeHook()
            {
                data().JmpHook.make_jmp(data().GetText, raw_ptr(GxtHook), false);
            }

            static void UnHook()
            {
                data().JmpHook.restore();
            }
            
    };

}
