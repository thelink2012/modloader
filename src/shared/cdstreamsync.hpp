#pragma once
#include <windows.h>
#include "CdStreamInfo.h"

struct CdStream;

namespace CdStreamSyncFix
{
	union SyncObj
	{
		HANDLE semaphore;
		CONDITION_VARIABLE cv;
	};

	struct SyncFuncs
	{
		SyncObj (__stdcall* Initialize)();
		void (__stdcall* Shutdown)( CdStream* stream );
		void (__stdcall* SleepCS)( CdStream* stream, PCRITICAL_SECTION critSec );
		void (__stdcall* Wake)( CdStream* stream );
	};

	namespace Sema
	{
		SyncObj __stdcall Initialize();
		void __stdcall Shutdown( CdStream* stream );
		void __stdcall SleepCS( CdStream* stream, PCRITICAL_SECTION critSec );
		void __stdcall Wake( CdStream* stream );
	}

	namespace CV
	{
		SyncObj __stdcall Initialize();
		void __stdcall Shutdown( CdStream* stream );
		void __stdcall SleepCS( CdStream* stream, PCRITICAL_SECTION critSec );
		void __stdcall Wake( CdStream* stream );
	}

	bool TryInitCV();

	inline SyncFuncs InitializeSyncFuncs()
	{
		SyncFuncs funcs;
		if ( TryInitCV() )
		{
			using namespace CV;
			funcs.Initialize = Initialize;
			funcs.Shutdown = Shutdown;
			funcs.SleepCS = SleepCS;
			funcs.Wake = Wake;
		}
		else
		{
			using namespace Sema;
			funcs.Initialize = Initialize;
			funcs.Shutdown = Shutdown;
			funcs.SleepCS = SleepCS;
			funcs.Wake = Wake;
		}
		return funcs;
	}
}

static_assert(sizeof(CdStreamSyncFix::SyncObj) == sizeof(HANDLE), "Incorrect struct size: CdStreamSync::SyncObj");