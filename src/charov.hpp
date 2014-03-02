/*
 *  Wide char overload for char functions
 *	(C) 2012 Denilson das Mercês Amorim <dma_2012@hotmail.com>
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
#ifndef _CHAROV_HPP_
#define _CHAROV_HPP_

#include <cstdio>
#include <cstdarg>
#include <cwchar>


#if defined _WIN32 && defined __GNUG__
	// on MinGW, check if have broken vswprintf
	#ifndef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
		#define CHAR_OVERLOAD_FIXED_BROKEN_SPRINTF
	#endif
#endif


namespace cwc
{

//=================================== stdio ===================================//

// testing WEOF\EOF
inline bool is_eof(wint_t c)
{
	return c == WEOF;
}

inline bool is_eof(wchar_t c)
{
	return c == WEOF;
}

inline bool is_eof(int c)
{
	return c == EOF;
}

inline bool is_eof(char c)
{
	return c == EOF;
}


// printf & scanf //
inline int vfprintf(FILE* stream, const wchar_t* format, va_list arg)
{
	return std::vfwprintf(stream, format, arg);
}


// sprintf is broken on windows, sorry for the incovenience
#if  !defined(_WIN32) || defined CHAR_OVERLOAD_USE_BROKEN_SPRINTF || defined CHAR_OVERLOAD_FIXED_BROKEN_SPRINTF	

#if !defined(_WIN32) || defined CHAR_OVERLOAD_FIXED_BROKEN_SPRINTF
	#define CHAR_OVERLOAD_SPRINTF_IS_OKAY
#endif


inline int vsnprintf(wchar_t* s, size_t len, const wchar_t* format, va_list arg)
{
#ifdef CHAR_OVERLOAD_SPRINTF_IS_OKAY
	return std::vswprintf(s, len, format, arg);
#else
	return ::vswprintf(s, format, arg);
#endif
}

inline int vsprintf(wchar_t* s, const wchar_t* format, va_list arg)
{
	return vsnprintf(s, -1, format, arg);
}

inline int snprintf(wchar_t* s, size_t n, const wchar_t* format, ...)
{
	va_list va; int x; va_start(va, format);
	x = vsnprintf(s, n, format, va);
	va_end(va);
	return x;
}

inline int sprintf(wchar_t* s, const wchar_t* format, ...)
{
	va_list va; int x; va_start(va, format);
	x = vsprintf(s, format, va);
	va_end(va);
	return x;
}

//#undef CHAR_OVERLOAD_SPRINTF_IS_OKAY
#endif // sprintf's

inline int vprintf(const wchar_t* format, va_list arg)
{
	return std::vwprintf(format, arg);	
}

inline int fprintf(FILE* stream, const wchar_t* format, ...)
{
	va_list va; int x; va_start(va, format);
	x = vfprintf(stream, format, va);
	va_end(va);
	return x;
}

inline int wprintf(const wchar_t* format, ...)
{
	va_list va; int x; va_start(va, format);
	x = vprintf(format, va);
	va_end(va);
	return x;
}


inline int vfscanf(FILE* stream, const wchar_t* format, va_list arg)
{
	return std::vfwscanf(stream, format, arg);
}

inline int vsscanf(const wchar_t* s, const wchar_t* format, va_list arg)
{
	return std::vswscanf(s, format, arg);
}

inline int vscanf(const wchar_t* format, va_list arg)
{
	return std::vwscanf(format, arg);
}

inline int fscanf(FILE* stream, const wchar_t* format, ... )
{
	va_list va; int x; va_start(va, format);
	x = vfscanf(stream, format, va);
	va_end(va);
	return x;
}

inline int sscanf(const wchar_t* s, const wchar_t* format, ...)
{
	va_list va; int x; va_start(va, format);
	x = vsscanf(s, format, va);
	va_end(va);
	return x;
}

inline int wscanf(const wchar_t* format, ...)
{
	va_list va; int x; va_start(va, format);
	x = vscanf(format, va);
	va_end(va);
	return x;
}



// (un)(put/get) char //
inline wint_t fgetc(FILE* stream)
{
	return std::fgetwc(stream);
}

inline wchar_t* fgets(wchar_t* str, int num, FILE* stream)
{
	return std::fgetws(str, num, stream);
}

inline wint_t fputc(wchar_t c, FILE* stream )
{
	return std::fputwc(c, stream);
}

inline int fputs(const wchar_t* str, FILE* stream)
{
	return std::fputws(str, stream);
}

inline wint_t getc(FILE* stream)
{
	return std::getwc(stream);
}

inline wint_t putc(wchar_t c, FILE* stream)
{
	return std::putwc(c, stream);
}


inline wint_t putchar(wchar_t c)
{
	return std::putwchar(c);
}

inline wint_t ungetc(wint_t c, FILE* stream)
{
	return std::ungetwc(c, stream);
}


//=================================== stdlib ===================================//
inline double strtod(const wchar_t* str, wchar_t** endptr)
{
	return wcstod(str, endptr);
}


inline float strtof(const wchar_t* str, wchar_t** endptr)
{
	return wcstof(str, endptr);
}

inline long double strtold(const wchar_t* str, wchar_t ** endptr)
{
	return wcstold(str, endptr);
}

inline long int strtol(const wchar_t* str, wchar_t** endptr, int base)
{
	return wcstol(str, endptr, base);
}

inline long long int strtoll(const wchar_t* str, wchar_t** endptr, int base)
{
	return wcstoll(str, endptr, base);
}

inline unsigned long int strtoul(const wchar_t* str, wchar_t** endptr, int base)
{
	return wcstoul(str, endptr, base);
}

inline unsigned long long int strtoull(const wchar_t* str, wchar_t** endptr, int base)
{
	return wcstoull(str, endptr, base);
}

//=================================== time ===================================//
inline size_t strftime(wchar_t* ptr, size_t maxsize, const wchar_t* format, const struct tm* timeptr)
{
	return wcsftime(ptr, maxsize, format, timeptr);
}



//=================================== string ===================================//

////////////// this library funcs //////////////////////////
template<class T, class U>
inline T* strcpy(T* dest, const U* src, T replace_invalid_chars = 0)
{
	T* p = dest;
	do
	{
		if(*src > 0x7F) *p = replace_invalid_chars;
		else *p = *src++;
	}
	while(*p++);
	return dest;
}

template<class T, class U>
inline T* strncpy(T* dest, const U* src, size_t num, T replace_invalid_chars = 0)
{
	T* p = dest;
	bool found_null = false;
	while(num-- != 0)
	{
		if(found_null)
		{
			*p = 0;
		}
		else
		{
			if(*src > 0x7F) *p = replace_invalid_chars;
			else *p = *src++;
			if(*p++ == 0) found_null = true;
		}
	}
	return dest;
}


template<class T, class U>
inline size_t strncmp(const T* a, const U* b, size_t num)
{
	while(num-- != 0)
	{
		if(*a != *b) return *a - *b;
		else if(*a == 0) return 0;
		++a, ++b;
	}
	return 0;
}

template<class T, class U>
inline size_t strcmp(const T* a, const U* b)
{
	return strncmp(a, b, (size_t)(-1));
}



////////////// standard //////////////////////////
inline wchar_t* strcpy(wchar_t* dest, const wchar_t* src)
{
	return wcscpy(dest, src);
}


inline size_t strcmp(const wchar_t* a, const wchar_t* b)
{
	return wcscmp(a, b);
}

inline wchar_t* strncpy(wchar_t* dest, const wchar_t* src, size_t num)
{
	return wcsncpy(dest, src, num);
}

inline size_t strncmp(const wchar_t* a, const wchar_t* b, size_t num)
{
	return wcsncmp(a, b, num);
}

inline wchar_t* strcat(wchar_t* destination, const wchar_t* source)	// <<
{
	return wcscat(destination, source);
}

inline const wchar_t* strchr(const wchar_t* s, wchar_t c)
{
	return wcschr(s, c);
}

inline wchar_t* strchr(wchar_t* s, wchar_t c)
{
	return wcschr(s, c);
}

inline int strcoll(const wchar_t* str1, const wchar_t* str2)
{
	return wcscoll(str1, str2);
}

inline size_t strcspn(const wchar_t* str1, const wchar_t* str2)
{
	return wcscspn(str1, str2);
}

inline size_t strlen(const wchar_t* str)
{
	return wcslen(str);
}

inline wchar_t* strncat(wchar_t* destination, wchar_t* source, size_t num)
{
	return wcsncat(destination, source, num);
}

inline const wchar_t* strpbrk(const wchar_t* str1, const wchar_t* str2)
{
	return wcspbrk(str1, str2);
}

inline wchar_t* strpbrk(wchar_t* str1, const wchar_t* str2)
{
	return wcspbrk(str1, str2);
}

inline const wchar_t* strrchr(const wchar_t* s, wchar_t c)
{
	return wcsrchr(s, c);
}

inline wchar_t* strrchr(wchar_t* s, wchar_t c)
{
	return wcsrchr(s, c);
}

inline size_t strspn(const wchar_t* str1, const wchar_t* str2)
{
	return wcsspn(str1, str2);
}

inline const wchar_t* strstr(const wchar_t* str1, const wchar_t* str2)
{
	return wcsstr(str1, str2);
}

inline wchar_t* strstr(wchar_t* str1, const wchar_t* str2)
{
	return wcsstr(str1, str2);
}

inline wchar_t* strtok(wchar_t* str, const wchar_t* delims)
{
	return wcstok(str, delims);
}

inline size_t strxfrm(wchar_t* destination, const wchar_t* source, size_t num)
{
	return wcsxfrm(destination, source, num);
}

#ifdef _WIN32

inline int stricmp(const wchar_t* a1, const wchar_t* a2)
{
    return wcsicmp(a1, a2);
}

inline int strnicmp(const wchar_t* a1, const wchar_t* a2, size_t num)
{
    return wcsnicmp(a1, a2, num);
}

#endif


//--------------------------------------------------------------------------
// NOT GOING TO OVERLOAD wmemXXX functions, makes non-sense in my mind
//--------------------------------------------------------------------------


}	// namespace
#endif	// include guard

