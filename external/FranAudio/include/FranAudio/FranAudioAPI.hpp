// FranticDreamer 2022-2025
#pragma once

// TODO: Change file extension to .h because this can also be used in C

// DLL Exporting API Defines
#if defined(_WIN32) || defined(_WIN64) // Windows
#ifdef FRANAUDIO_DYNAMIC
#define FRANAUDIO_API __declspec(dllexport)
#elif defined FRANAUDIO_IGNORE
#define FRANAUDIO_API
#else
#define FRANAUDIO_API __declspec(dllimport)
#endif
#else // Unix
#ifdef FRANAUDIO_DYNAMIC
#define FRANAUDIO_API __attribute__((visibility("default")))
#else
#define FRANAUDIO_API
#endif
#endif

// If FranAudio will run on it's separate process
// We can use IPC actions.
#ifdef FRANAUDIO_USE_SERVER
// TODO: Different Defines?
#endif
