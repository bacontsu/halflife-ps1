// FranticDreamer 2022-2026

// Water shader setting defaults for func_water.
// Shared between the server entity (dlls/doors.cpp) and the client's water shader (cl_dll/renderer/watershader.h)

#ifndef WATERSHADERDEFS_H
#define WATERSHADERDEFS_H

constexpr int WATER_DEFAULT_FOG_START = 100;
constexpr int WATER_DEFAULT_FOG_END = 400;

constexpr int WATER_DEFAULT_COLOR_R = 64;
constexpr int WATER_DEFAULT_COLOR_G = 80;
constexpr int WATER_DEFAULT_COLOR_B = 90;

constexpr float WATER_DEFAULT_FRESNEL = 1.0f;

// Waves are off by default (WaveHeight 0).
// Frequency is in radians per unit, speed in radians per second.
constexpr float WATER_DEFAULT_WAVE_FREQ = 0.05f;
constexpr float WATER_DEFAULT_WAVE_SPEED = 1.0f;

#endif // WATERSHADERDEFS_H
