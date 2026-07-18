// FranticDreamer 2022-2025
#pragma once

// TODO: Change file extension to .h because this can also be used in C

// DLL Exporting API Defines
#if defined(_WIN32) || defined(_WIN64) // Windows
#ifdef FRANAUDIO_CLIENT_DYNAMIC
#define FRANAUDIO_CLIENT_API __declspec(dllexport)
#else
#define FRANAUDIO_CLIENT_API __declspec(dllimport)
#endif
#else // Unix
#ifdef FRANAUDIO_CLIENT_DYNAMIC
#define FRANAUDIO_CLIENT_API __attribute__((visibility("default")))
#else
#define FRANAUDIO_CLIENT_API
#endif
#endif

