#pragma once

// the NOINLINE macro tells the compiler not to inline functions.
// this is primarily used to make modding easier.
#ifndef NOINLINE
	#if defined(__GNUC__) || defined(__GNUG__)
		#define NOINLINE __attribute__((noinline))
	#elif defined(_MSC_VER)
		#define NOINLINE __declspec(noinline)
	#else
		#define NOINLINE 
	#endif
#endif // NOINLINE