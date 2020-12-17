#ifndef __DEFINES_H__
#define __DEFINES_H__

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif __unix__
#define EXPORT
#endif

#endif // __DEFINES_H__