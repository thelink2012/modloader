/* 
 * San Andreas modloader
 * Copyright (C) 2013-2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#ifndef ADDRESS_TRANSLATOR_H
#define	ADDRESS_TRANSLATOR_H

#define INJECTOR_USE_VARIADIC_TEMPLATES
#define INJECTOR_GVM_HAS_TRANSLATOR

#include <injector/injector.hpp>
#include <injector/hooking.hpp>
#include <map>

namespace modloader
{
    using injector::memory_pointer_raw;
    using injector::address_manager;
    using injector::raw_ptr;
    
    /*
     *  AddressTranslator
     *      Translates 10US addresses into other executables addresses 
     */
    class AddressTranslator
    {
        private:
            static const size_t ptr_failure  = 0x2012;  // Useful to know the reason for a crash on accessing 0x2012
            static const size_t max_ptr_dist = 8;       // Max distance to take as a "equivalent" address for modloader

            typedef std::map<memory_pointer_raw, memory_pointer_raw> map_type;

            // Address table
            static map_type& get_map()
            { static map_type m; return m; }

            // Game version manager object
            static address_manager& get_gvm()
            { return injector::address_manager::singleton(); }

            // Initializes the address translator and it's table
            static bool init()
            {
                static bool bInitialized = false;
                if(bInitialized == false)
                {
                    auto& gvm = get_gvm();
                    if(true)
                    {
                        if(gvm.IsSA())  // We're only working with SA addresses on here
                        {
                            // Find version and initialize addresses table
                            if(gvm.GetMajorVersion() == 1 && gvm.GetMinorVersion() == 0 && gvm.IsUS())
                                init_10us();
                            
                            // The map must have null pointers at it's bounds
                            // So they work properly with lower_bound and stuff
                            get_map().emplace(0x00000000u, 0x00000000u);
                            get_map().emplace(0xffffffffu, 0xffffffffu);
                        }
                    }
                }
                return bInitialized;
            }

        public:
            // Translates address @p for the current executable
            static memory_pointer_raw translate(memory_pointer_raw p)
            {
                // Initialize if hasn't initialized yet
                init();
                
                //
                auto& map = get_map();
                memory_pointer_raw result = nullptr;
                
                // Find first element in the map that is greater than or equal to @p
                auto it = map.lower_bound(p);
                if(it != map.end())
                {
                    // If it's not exactly the address, get back one position on the table
                    if(it->first != p) --it;

                    auto diff = uintptr_t(p - it->first);   // What's the difference between @p and that address?
                    if(diff <= max_ptr_dist)                // Could we live with this difference in hands?
                        result = it->second + raw_ptr(diff);         // Yes, we can!

                }
                
                // Print into stdout that the address translation wasn't possible
                if(!result)
                {
                    printf("modloader: Could not translate address 0x%p!!\n", p.get<void>());
                    result = ptr_failure;
                }
                
                return result;
            }
            
        private:
            // Initializes 10US table
            static void init_10us()
            {
                auto& map = get_map();
                bool isHoodlum = get_gvm().IsHoodlum();
                
                map[0x00406560] = 0x00406560;
                map[0x00406a20] = 0x00406a20;
                map[0x00407614] = 0x00407614;
                map[0x0040cca6] = 0x0040cca6;
                map[0x0040cf34] = 0x0040cf34;
                map[0x00459ff2] = 0x00459ff2;
                map[0x0045a002] = 0x0045a002;
                map[0x00468eb6] = 0x00468eb6;
                map[0x00468ec5] = 0x00468ec5;
                map[0x004706f0] = 0x004706f0;
                map[0x00489a46] = 0x00489a46;
                map[0x0049ea9e] = 0x0049ea9e;
                map[0x004d563e] = 0x004d563e;
                map[0x005322f0] = 0x005322f0;
                map[0x00532350] = 0x00532350;
                map[0x0053c6db] = 0x0053c6db;
                map[0x0053e58e] = 0x0053e58e;
                map[0x00590d00] = 0x00590d00;
                map[0x00590d2b] = 0x00590d2b;
                map[0x00590d68] = 0x00590d68;
                map[0x005a69e4] = 0x005a69e4;
                map[0x005a69e9] = 0x005a69e9;
                map[0x005a69ee] = 0x005a69ee;
                map[0x005a69f8] = 0x005a69f8;
                map[0x005b6170] = 0x005b6170;
                map[0x005b6183] = 0x005b6183;
                map[0x005b61b8] = 0x005b61b8;
                map[0x005b61e1] = 0x005b61e1;
                map[0x005b63e8] = 0x005b63e8;
                map[0x005b6419] = 0x005b6419;
                map[0x005b6449] = 0x005b6449;
                map[0x005b65be] = 0x005b65be;
                map[0x005b8428] = 0x005b8428;
                map[0x005b871a] = 0x005b871a;
                map[0x005b8e1b] = 0x005b8e1b;
                map[0x005b905e] = 0x005b905e;
                map[0x005b915b] = 0x005b915b;
                map[0x005bbacb] = 0x005bbacb;
                map[0x005bbade] = 0x005bbade;
                map[0x005bc09b] = 0x005bc09b;
                map[0x005bc0ae] = 0x005bc0ae;
                map[0x005bd839] = 0x005bd839;
                map[0x005bd84c] = 0x005bd84c;
                map[0x005bd850] = 0x005bd850;
                map[0x005c248f] = 0x005c248f;
                map[0x005dd25d] = 0x005dd25d;
                map[0x0069fce2] = 0x0069fce2;
                map[0x0069fd5a] = 0x0069fd5a;
                map[0x006a0050] = 0x006a0050;
                map[0x006a01bf] = 0x006a01bf;
                map[0x006a0228] = 0x006a0228;
                map[0x00748c30] = 0x00748c30;
                map[0x008246ec] = 0x008246ec;
                map[0x008e3fe0] = 0x008e3fe0;
                map[0x008e3fec] = 0x008e3fec;
                map[0x008e4cc0] = 0x008e4cc0;
                map[0x005900cc] = 0x005900cc;
                map[0x005900b6] = 0x005900b6;
                map[0x0040a106] = !isHoodlum? 0x0040a106 : 0x0156644f;
                map[0x00406886] = !isHoodlum? 0x00406886 : 0x01564b90;
                map[0x00407642] = !isHoodlum? 0x00407642 : 0x01567bc2;
                map[0x00406a5b] = !isHoodlum? 0x00406a5b : 0x0156c2fb;
                map[0x00409f76] = 0x00409f76;
                map[0x00409fd9] = 0x00409fd9;
                map[0x0048418a] = 0x0048418a;
                map[0x005b630b] = 0x005b630b;
                map[0x005A6A01] = 0x005A6A01;
                map[0x008E3ED4] = 0x008E3ED4;
                map[0x00820EBA] = 0x00820EBA;
                
                // ^ The above table must be restructured
                // Pointers there may be repeating below:
                
                // std-asi
                map[0x836F3B] = 0x836F3B;   // chdir return ptr
                
                // FX pointers
                map[0x5B8F58] = 0x5B8F58;
                map[0x49EA9D] = 0x49EA9D;
                map[0x5BA69E] = 0x5BA69E;
                map[0x572F18] = 0x572F18;
                map[0x57313C] = 0x57313C;
                map[0x57303A] = 0x57303A;
                map[0x5731FB] = 0x5731FB;
                map[0x572FAF] = 0x572FAF;
                map[0x5BA85F] = 0x5BA85F;
                map[0x5BF8B1] = 0x5BF8B1;
                map[0x5BA7CE] = 0x5BA7CE;
                map[0x5DD959] = 0x5DD959;
                map[0x5C248F] = 0x5C248F;
                map[0x5DDA83] = 0x5DDA83;
                map[0x5DDA8B] = 0x5DDA8B;
                map[0x5DDA93] = 0x5DDA93;
                map[0x5DDA9B] = 0x5DDA9B;
                map[0x5DDABF] = 0x5DDABF;
                map[0x5DDAC7] = 0x5DDAC7;
                map[0x5DDACF] = 0x5DDACF;
                map[0x5DDAD7] = 0x5DDAD7;
                    
               // Movies pointers
                map[0x748AFA] = 0x748AFA;
                map[0x748BEC] = 0x748BEC;
                map[0x748BF3] = 0x748BF3;
                
                // Data pointers
                map[0x5BC09A] = 0x5BC09A;
                map[0x5BBACA] = 0x5BBACA;
                map[0x5BD838] = 0x5BD838;
                map[0x5DD3BA] = 0x5DD3BA;
                map[0x5BD84B] = 0x5BD84B;
                map[0x5DD75F] = 0x5DD75F;
                map[0x5B8428] = 0x5B8428;
                map[0x5B871A] = 0x5B871A;
                map[0x5B905E] = 0x5B905E;
                map[0x5BD850] = 0x5BD850;
                map[0x5B65BE] = 0x5B65BE;
                map[0x5BBADE] = 0x5BBADE;
                map[0x5BC0AE] = 0x5BC0AE;
                map[0x5DD3D1] = 0x5DD3D1;
                map[0x6EAF4D] = 0x6EAF4D;
                map[0x461125] = 0x461125;
                map[0x6F7470] = 0x6F7470;
                map[0x6F74BC] = 0x6F74BC;
                map[0x6F7496] = 0x6F7496;
                map[0x6F74E2] = 0x6F74E2;
                map[0x7187DB] = 0x7187DB;
                
                // Streaming pointers
                map[0x4D563D] = 0x4D563D;
                map[0x408430] = 0x408430;
                map[0x406C2A] = 0x406C2A;
                map[0x40844C] = 0x40844C;
                map[0x40846E] = 0x40846E;
                map[0x40848C] = 0x40848C;
                map[0x5A41A4] = 0x5A41A4;
                map[0x5A69F7] = 0x5A69F7;
                map[0x5A80F9] = 0x5A80F9;
                map[0x4D5EB9] = 0x4D5EB9;
                map[0x5AFBCB] = 0x5AFBCB;
                map[0x5AFC98] = 0x5AFC98;
                map[0x5B07DA] = 0x5B07DA;
                map[0x5B1423] = 0x5B1423;
                map[0x4083E4] = 0x4083E4;
                map[0x407610] = 0x407610;
                map[0x8E3FEC] = 0x8E3FEC;
                map[0x40844C] = 0x40844C;
                map[0x40848C] = 0x40848C;
                
                // Bank loader
                map[0x5B9D68] = 0x5B9D68;
                map[0x4E0597] = 0x4E0597;
                map[0x4DFBD7] = 0x4DFBD7;
                map[0x4DFC7D] = 0x4DFC7D;
                map[0x4D99B3] = 0x4D99B3;
                map[0x4D9800] = 0x4D9800;
                map[0x4DFE30] = 0x4DFE30;
                map[0x4E065B] = 0x4E065B;
                map[0x4DFD9D] = 0x4DFD9D;
                map[0x4DFDC3] = 0x4DFDC3;
                map[0x4DFDCE] = 0x4DFDCE;
                
                // Track loader
                map[0x4E0E25] = 0x4E0E25;
                map[0x4E0982] = 0x4E0982;
                map[0x4E0A02] = 0x4E0A02;
                map[0x4E0DA2] = 0x4E0DA2;
                map[0x4E0AF2] = 0x4E0AF2;
                map[0x4E0DA7] = 0x4E0DA7;
                map[0x4E0AF7] = 0x4E0AF7;
                
            }

    };

}

// Translate pointer from 10US offset to this executable offset
inline void* injector::address_manager::translator(void* p)
{
    return modloader::AddressTranslator::translate(p).get();
}




#endif

