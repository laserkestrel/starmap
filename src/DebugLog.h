// DebugLog.h
#pragma once
#include <iostream>

// Debug logging macro: enabled for debug builds. It is active when either
// `_DEBUG` is defined (MSVC/GCC debug) or when `NDEBUG` is not defined
// (standard C release macro). In production builds where `NDEBUG` is defined
// this compiles out to an empty statement.
#if defined(_DEBUG) || !defined(NDEBUG)
#define DEBUG_LOG(x) do { std::cout << x << std::endl; } while(0)
#else
#define DEBUG_LOG(x) do { } while(0)
#endif
