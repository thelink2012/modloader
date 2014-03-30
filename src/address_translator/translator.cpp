/* 
 * San Andreas Mod Loader - Address Translation Between Game Versions
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

using namespace injector;

// Constants
static const size_t ptr_failure  = 0x2012;  // Useful to know the reason for a crash on accessing 0x2012
static const size_t max_ptr_dist = 8;       // Max distance to take as a "equivalent" address for modloader

// Forwarding
static void init();
static void sa_10us();

// Addresses table
static std::map<memory_pointer_raw, memory_pointer_raw> map;


// Translate pointer from 10US offset to this executable offset
void* injector::address_manager::translator(void* p_)
{
    memory_pointer_raw p = p_;
    memory_pointer_raw result = nullptr;
                
    // Initialize if hasn't initialized yet
    init();

    // Find first element in the map that is greater than or equal to p
    auto it = map.lower_bound(p);
    if(it != map.end())
    {
        // If it's not exactly the address, get back one position on the table
        if(it->first != p) --it;

        auto diff = uintptr_t(p - it->first);       // What's the difference between p and that address?
        if(diff <= max_ptr_dist)                    // Could we live with this difference in hands?
            result = it->second + raw_ptr(diff);    // Yes, we can!
    }
    
    // Print into stdout that the address translation wasn't possible
    // This should never happen! The user won't see this warning on stdout.
    if(!result)
    {
        printf("Could not translate address 0x%p!!\n", p.get<void>());
        result = ptr_failure;
    }
    
    return result.get();
}

// Initializes the address translator and it's table
static void init()
{
    static bool bInitialized = false;
    if(bInitialized == false)
    {
        auto& gvm = injector::address_manager::singleton();
        bInitialized = true;
        
        // We're only working with SA addresses on here
        if(gvm.IsSA())
        {
            // Find version and initialize addresses table
            if(gvm.GetMajorVersion() == 1 && gvm.GetMinorVersion() == 0 && gvm.IsUS())
                sa_10us();

            // The map must have null pointers at it's bounds
            // So they work properly with lower_bound and stuff
            map.emplace(0x00000000u, 0x00000000u);
            map.emplace(0xffffffffu, 0xffffffffu);
        }
    }
}



/*
 *  ---------------> Address Tables
 */

// 10US table
static void sa_10us()
{
    bool isHoodlum = injector::address_manager::singleton().IsHoodlum();
    
    // Core
    map[0x8246EC] = 0x8246EC;   // call    _WinMain
    map[0x748C30] = 0x748C30;   // call    CGame::InitialiseEssentialsAfterRW
    map[0x53E58E] = 0x53E58E;   // call    CGame::Initialize
    map[0x53C6DB] = 0x53C6DB;   // call    CGame::ReInitObjectVariables
    map[0x590D00] = 0x590D00;   // CLoadingScreen::NewChunkLoaded
    map[0x590D2A] = 0x590D2A;   // cmp     eax, 8Ch     ; Max Bar Size
    map[0x590D67] = 0x590D67;   // cmp     eax, 8Ch     ; Max Bar Size
    
    // ASI Injector
    map[0x836F3B] = 0x836F3B;   // SetCurrentDirectory return pointer for _chdir
    
    // Movies pointers
    map[0x748AFA] = 0x748AFA;   // push    offset "movies\\Logo.mpg"
    map[0x748BEC] = 0x748BEC;   // push    offset "movies\\GTAtitles.mpg"
    map[0x748BF3] = 0x748BF3;   // push    offset "movies\\GTAtitlesGER.mpg"
    
    // Text
    map[0x6A0228] = 0x6A0228;   // call    CFileMgr::Open
    map[0x69FD5A] = 0x69FD5A;   // call    CFileMgr::Open
    map[0x6A0050] = 0x6A0050;   // CText::Get
    
    // Sprites
    map[0x5900CC] = 0x5900CC;   // push    offset "loadscs.txd"
    map[0x48418A] = 0x48418A;   // call    CTxdStore::LoadTxd
    
    // SCM
    map[0x468EB5] = 0x468EB5;   // push    offset "data\\script"
    map[0x468EC4] = 0x468EC4;   // push    offset "main.scm"
    map[0x489A45] = 0x489A45;   // push    offset "data\\script\\main.scm"

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
    map[0x5DD25C] = 0x5DD25C;
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
    map[0x406560] = 0x406560;
    map[0x406A20] = 0x406A20;
    map[0x407613] = 0x407613;
    map[0x40CCA6] = 0x40CCA6;
    map[0x40CF34] = 0x40CF34;
    map[0x459FF0] = 0x459FF0;
    map[0x45A001] = 0x45A001;
    map[0x4706F0] = 0x4706F0;
    map[0x5322F0] = 0x5322F0;
    map[0x532350] = 0x532350;
    map[0x5A69E3] = 0x5A69E3;
    map[0x5A69E8] = 0x5A69E8;
    map[0x5A69ED] = 0x5A69ED;
    map[0x5B6170] = 0x5B6170;
    map[0x5B6183] = 0x5B6183;
    map[0x5B61B8] = 0x5B61B8;
    map[0x5B61E1] = 0x5B61E1;
    map[0x5B63E8] = 0x5B63E8;
    map[0x5B6419] = 0x5B6419;
    map[0x5B6449] = 0x5B6449;
    map[0x5B8E1B] = 0x5B8E1B;
    map[0x5B915B] = 0x5B915B;
    map[0x8E3FE0] = 0x8E3FE0;
    map[0x8E4CC0] = 0x8E4CC0;
    map[0x40A106] = !isHoodlum? 0x40A106 : 0x0156644F;
    map[0x406886] = !isHoodlum? 0x406886 : 0x01564B90;
    map[0x407642] = !isHoodlum? 0x407642 : 0x01567BC2;
    map[0x406A5B] = !isHoodlum? 0x406A5B : 0x0156C2FB;
    map[0x409F76] = 0x409F76;
    map[0x409FD9] = 0x409FD9;
    map[0x5B630B] = 0x5B630B;
    map[0x5A6A01] = 0x5A6A01;
    
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


#endif

