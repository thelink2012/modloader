#pragma once
#include "cdstreamsync.hpp"

namespace CdStreamSyncFix
{
	namespace Sema
	{
		inline SyncObj __stdcall Initialize()
		{
			SyncObj object;
			object.semaphore = CreateSemaphore( nullptr, 0, 2, nullptr );
			return object;
		}

		inline void __stdcall Shutdown( CdStream* stream )
		{
			CloseHandle( stream->sync.semaphore );
		}

		inline void __stdcall SleepCS( CdStream* stream, PCRITICAL_SECTION critSec )
		{
			if ( stream->nSectorsToRead != 0 )
			{
				stream->bLocked = 1;
				LeaveCriticalSection( critSec );
				WaitForSingleObject( stream->sync.semaphore, INFINITE );
				EnterCriticalSection( critSec );
			}
		}

		inline void __stdcall Wake( CdStream* stream )
		{
			if( stream->bLocked ) ReleaseSemaphore( stream->sync.semaphore, 1, nullptr );
		}
	}

	namespace CV
	{
		namespace Funcs
		{
			static decltype(InitializeConditionVariable)* pInitializeConditionVariable = nullptr;
			static decltype(SleepConditionVariableCS)* pSleepConditionVariableCS = nullptr;
			static decltype(WakeConditionVariable)* pWakeConditionVariable = nullptr;
		}

		inline SyncObj __stdcall Initialize()
		{
			SyncObj object;
			Funcs::pInitializeConditionVariable( &object.cv );
			return object;
		}

		inline void __stdcall Shutdown( CdStream* stream )
		{
		}

		inline void __stdcall SleepCS( CdStream* stream, PCRITICAL_SECTION critSec )
		{
			while ( stream->nSectorsToRead != 0 )
			{
				Funcs::pSleepConditionVariableCS( &stream->sync.cv, critSec, INFINITE );
			}
		}

		inline void __stdcall Wake( CdStream* stream )
		{
			Funcs::pWakeConditionVariable( &stream->sync.cv );
		}
	}

	bool TryInitCV()
	{
		const HMODULE kernelDLL = GetModuleHandle( TEXT("kernel32") );
		assert( kernelDLL != nullptr );

		using namespace CV::Funcs;
		pInitializeConditionVariable = (decltype(pInitializeConditionVariable))GetProcAddress( kernelDLL, "InitializeConditionVariable" );
		pSleepConditionVariableCS = (decltype(pSleepConditionVariableCS))GetProcAddress( kernelDLL, "SleepConditionVariableCS" );
		pWakeConditionVariable = (decltype(pWakeConditionVariable))GetProcAddress( kernelDLL, "WakeConditionVariable" );

		return pInitializeConditionVariable != nullptr && pSleepConditionVariableCS != nullptr && pWakeConditionVariable != nullptr;
	}
}