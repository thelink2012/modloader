/* 
 * San Andreas Mod Loader Exception Description
 * Created by LINK/2012 <dma_2012@hotmail.com>
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
#include <windows.h>
#include <cstdio>

/*
 * GetExceptionCodeString
 *      Returns an description by an exception code 
 */
 static const char* GetExceptionCodeString(unsigned int code)
 {
      switch(code)
      {
         case EXCEPTION_ACCESS_VIOLATION:         return "Access violation";
         case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "Array bounds exceeded";
         case EXCEPTION_BREAKPOINT:               return "Breakpoint exception";
         case EXCEPTION_DATATYPE_MISALIGNMENT:    return "Data type misalignment exception";
         case EXCEPTION_FLT_DENORMAL_OPERAND:     return "Denormal float operand";
         case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "Floating-point division by zero";
         case EXCEPTION_FLT_INEXACT_RESULT:       return "Floating-point inexact result";
         case EXCEPTION_FLT_INVALID_OPERATION:    return "Floating-point invalid operation";
         case EXCEPTION_FLT_OVERFLOW:             return "Floating-point overflow";
         case EXCEPTION_FLT_STACK_CHECK:          return "Floating-point stack check";
         case EXCEPTION_FLT_UNDERFLOW:            return "Floating-point underflow";
         case EXCEPTION_ILLEGAL_INSTRUCTION:      return "Illegal instruction.";
         case EXCEPTION_IN_PAGE_ERROR:            return "In page error";
         case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "Integer division by zero";
         case EXCEPTION_INT_OVERFLOW:             return "Integer overflow";
         case EXCEPTION_INVALID_DISPOSITION:      return "Invalid disposition";
         case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Non-continuable exception";
         case EXCEPTION_PRIV_INSTRUCTION:         return "Privileged instruction";
         case EXCEPTION_SINGLE_STEP:              return "Single step exception";
         case EXCEPTION_STACK_OVERFLOW:           return "Stack overflow";
         default:                                 return "NO_DESCRIPTION";
      }
}

/*
 * LogException
 *      Takes an LPEXCEPTION_POINTERS and transforms in a string that is put in the logging steam
 */
int LogException(char* buffer, LPEXCEPTION_POINTERS pException)
{
    buffer[0] = 0;
    size_t len = 0;
    char szModuleName[MAX_PATH];
    MEMORY_BASIC_INFORMATION mbi;
    LPEXCEPTION_RECORD pRecord;

    
    
    // Acquiere common information that we'll access
    pRecord = pException->ExceptionRecord;
    DWORD dwExceptionCode = pRecord->ExceptionCode;
    
    // Findout the module that the exception address is at...
    if(VirtualQuery(pRecord->ExceptionAddress, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        // ...and findout that module name.
        if(GetModuleFileNameA((HMODULE)mbi.AllocationBase, szModuleName, sizeof(szModuleName)))
        {
            // Log the exception in a similar format similar to debuggers format
            len += sprintf(&buffer[len], "\nUnhandled exception at 0x%p in \"%s\": 0x%X: %s",
                          pRecord->ExceptionAddress, szModuleName, dwExceptionCode,
                          GetExceptionCodeString(dwExceptionCode));
            
            // If exception is IN_PAGE_ERROR or ACCESS_VIOLATION, we have additional information such as an address
            if(dwExceptionCode == EXCEPTION_IN_PAGE_ERROR || dwExceptionCode == EXCEPTION_ACCESS_VIOLATION)
            {
                DWORD rw       = (DWORD) pRecord->ExceptionInformation[0];  // read or write?
                ULONG_PTR addr = (DWORD) pRecord->ExceptionInformation[1];  // which address?
                
                len += sprintf(&buffer[len], " %s 0x%p",
                               rw == 0? "reading location" : rw == 1? "writing location" : rw == 8? "DEP at" : "",
                               addr);
                    
                // IN_PAGE_ERROR have another information...
                if(dwExceptionCode == EXCEPTION_IN_PAGE_ERROR)
                {
                    len += sprintf(&buffer[len], ".\nUnderlying NTSTATUS code that resulted in the exception is 0x%p",
                                   pRecord->ExceptionInformation[2]);
                }
            }

            // Terminate the log with a period
            len += sprintf(&buffer[len], ".");
        }
    }
    
    return buffer[0] != 0;
}


