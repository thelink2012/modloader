/*
 * GameInfo:
 * 		Class to detect the game, version and region;
 * 		Note:
 * 			[*] The object must be global if you are planning to use DelayedDetect function
 * 			[*]	If the game is SA and it is a non-cracked version and you are trying to detect
 * 				the game before the securom decryption occurs (DllMain), the class won't be able to detect infos,
 * 				a solution is to use DelayedDetect method
 * 
 * 
 * 	by LINK/2012 <dma_2012@hotmail.com>; Thanks to Silent!
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
#include "Injector.h"
#include <windows.h>			// for threads
#include <cstdio>				// for sprintf
#include <cassert>				// for assert (oh rly)


class GameInfo
{
	public:
		// Set this if you would like that MessagesBox contain PluginName as caption
		const char* PluginName;
		
	public:
		typedef void (*DelayedCallback)(GameInfo& info);

		enum Game
		{
			GAME_UNK, III, VC, SA
		};

		enum Region
		{
			REGION_UNK, US, EURO
		};

	private:
		struct DelayedInfo
		{
			GameInfo* info;
			DelayedCallback cb;
			bool running;
			bool raise;

		} delayed_info;

	private:
		Game	game;
		Region	region;
		int		major, minor;
		bool	steam;
		bool    delayed;

	public:
		GameInfo(Game xgame = GAME_UNK)
			: game(xgame), major(0), minor(0), steam(false), delayed(false)
		{
			delayed_info.running = false;
		}

		// Checks if I don't know the game we are attached to
		bool IsUnknown()		{ return game == GAME_UNK; }
		// Checks if this is the steam version
		bool IsSteam()			{ return steam; }
		// Checks if the detect was delayed
		bool IsDelayed()		{ return delayed; }
		// Gets the game we are attached to
		Game GetGame()			{ return game; }
		// Gets the region from the game we are attached to
		Region GetRegion()		{ return region; }
		// Get major and minor version of the game (e.g. [major = 1, minor = 0] = 1.0)
		int GetMajorVersion()	{ return major; }
		int GetMinorVersion()	{ return minor; }


		// Detects game, region and version; returns false if could not detect it
		bool Detect()
		{
			game = GAME_UNK, region = REGION_UNK;
			major = minor = 0;
			steam = false;

			// Look for game and version thought the entry-point
			return (DetectIII() || DetectVC() || DetectSA());
		}

		// Detects game version and region for this->game;
		// returns false if could not detect it or no game is assigned to the object
		bool DetectGameVersion()
		{
			if(game == III)
				return DetectIII();
			else if(game == VC)
				return DetectVC();
			else if(game == SA)
				return DetectSA();
			else
				return false;
		}

		// Asynchronous detects infos after the executable has been decrypted and calls callback when that occurs
		// Do not call this function more than once
		//
		//	 If the game could be detected without delay, it returns false and calls the callback with
		//   the object containing information about that game.
		//	 Otherwise it creates a thread (or not) and waits the decryption, if it takes too long to decrypt
		//   (because the game version is unknown or it is really slow), raises a error if bRaiseErrorOnFailure is true
		//
		bool DelayedDetect(DelayedCallback cb, bool bRaiseErrorOnFailure = true)
		{
			// Throw a brick on the programmer if running DelayedDetect more than once
			assert(delayed_info.running == false);
			delayed_info.running = true;

			if(Detect())
			{
				cb(*this);
				return (delayed = false);
			}

			delayed_info.info = this;
			delayed_info.cb = cb;
			delayed_info.raise = bRaiseErrorOnFailure;
			if(!CreateThread(0, 0, &DelayedDetectThread, &delayed_info, 0, 0))
			{
				// Oh man...
				RaiseCouldNotDetect();
			}

			return (delayed = true);
		}

		// Gets the game version as text, the buffer must contain at least 32 bytes of space.
		char* GetVersionText(char* buffer)
		{
			const char *g, *r;

			switch(game)
			{
				case III: g = "III";  break;
				case VC:  g = "VC";   break;
				case SA:  g = "SA";   break;
				default:  g = "UNK";  break;
			}

			switch(region)
			{
				case US:	r = "US";			break;
				case EURO:	r = "EURO";			break;
				default:	r = "UNK_REGION";	break;
			}

			sprintf(buffer, "GTA %s %d.%d%s", g, major, minor, r);
			return buffer;
		}


	public:
		// Raises a error saying that you could not detect the game version
		void RaiseCouldNotDetect()
		{
			MessageBoxA(0,
				"Could not detect the game version\nContact the mod creator!",
				PluginName, MB_ICONERROR
			);
		}

		// Raises a error saying that the exe version is incompatible (and output the exe name)
		void RaiseIncompatibleVersion()
		{
			char buf[128], v[32];
			sprintf(buf,
				"An incompatible exe version has been detected! (%s)\nContact the mod creator!",
				GetVersionText(v)
				);
			MessageBoxA(0, buf, PluginName, MB_ICONERROR);
		}


	private:
		// Thread body for delayed detection
		static DWORD _stdcall DelayedDetectThread(void* data)
		{
			int cycles = 0;
			DelayedInfo* d = (DelayedInfo*)(data);

			while(true)
			{
				Sleep(800);
				if( (d->info->game != GAME_UNK? d->info->DetectGameVersion() : d->info->Detect()) )
					break;
				if(++cycles >= 19 && d->raise)	// ~15 seconds has been passed
				{
					d->info->RaiseCouldNotDetect();
					return 1;
				}
			}

			d->cb(*d->info);
			return 0;
		}

		// Thanks Silent for the DetectXXX() function
		// DetectXXX():
		//		The DetectXXX Methods tries to detect the region and version for game XXX
		//		If it could not be detect, no changes are made to the object and the func returns false

		bool DetectIII()
		{
			if(ReadMemory<uint32_t>(0x5C1E70, true) == 0x53E58955)
				game = III, major = 1, minor = 0, region = REGION_UNK, steam = false;
			else if(ReadMemory<uint32_t>(0x5C2130, true) == 0x53E58955)
				game = III, major = 1, minor = 1, region = REGION_UNK, steam = false;
			else if(ReadMemory<uint32_t>(0x5C6FD0, true) == 0x53E58955)
				game = III, major = 1, minor = 1, region = REGION_UNK, steam = true;
			else
				return false;

			return true;
		}

		bool DetectVC()
		{
			if(ReadMemory<uint32_t>(0x667BF0, true) == 0x53E58955)
				game = VC, major = 1, minor = 0, region = REGION_UNK, steam = false;
			else if(ReadMemory<uint32_t>(0x667C40, true) == 0x53E58955)
				game = VC, major = 1, minor = 1, region = REGION_UNK, steam = false;
			else if(ReadMemory<uint32_t>(0xA402ED, true) == 0x56525153)
				game = VC, major = 1, minor = 1, region = REGION_UNK, steam = true;		// You have to check if steam
			else
				return false;

			return true;
		}

		bool DetectSA()
		{
			if(ReadMemory<uint32_t>(0x82457C, true) == 0x94BF)
				game = SA, major = 1, minor = 0, region = US, steam = false;
			else if(ReadMemory<uint32_t>(0x8245BC, true) == 0x94BF)
				game = SA, major = 1, minor = 0, region = EURO, steam = false;
			else if(ReadMemory<uint32_t>(0x8252FC, true) == 0x94BF)
				game = SA, major = 1, minor = 1, region = US, steam = false;
			else if(ReadMemory<uint32_t>(0x82533C, true) == 0x94BF)
				game = SA, major = 1, minor = 1, region = EURO, steam = false;
			else if(ReadMemory<uint32_t>(0x85EC4A, true) == 0x94BF)
				game = SA, major = 3, minor = 0, region = REGION_UNK, steam = true;
			else
				return false;

			return true;
		}


};
