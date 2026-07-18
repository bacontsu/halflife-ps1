// FranticDreamer 2022-2025
#pragma once

#include <memory>

#include "FranAudioAPI.hpp"

#include "Backend/Backend.hpp"

#include "FranAudioShared/Logger/Logger.hpp"

/// <summary>
/// FranAudio Library Root Namespace.
/// </summary>
namespace FranAudio
{
	/// <summary>
	/// Default backend for FranAudio.
	/// </summary>
	inline constexpr Backend::BackendType defaultBackend = Backend::BackendType::miniaudio;

	/// <summary>
	/// Global data for the FranAudio library.
	/// </summary>
	class GlobalData
	{
	public:
		std::unique_ptr<FranAudio::Backend::Backend> currentBackend;
	};

	/// <summary>
	/// Global instance of the FranAudio library data.
	/// </summary>
	extern GlobalData gGlobals;

	/// <summary>
	/// Initializes the FranAudio library.
	/// </summary>
	FRANAUDIO_API void Init();

	/// <summary>
	/// Resets the FranAudio library.
	/// This is used to reset the library to its initial state.
	/// </summary>
	FRANAUDIO_API void Reset();

	/// <summary>
	/// Shuts down the FranAudio library and cleans up resources.
	/// </summary>
	FRANAUDIO_API void Shutdown();

	/// <summary>
	/// Routes the library logging output to the specified console stream buffer.
	/// </summary>
	/// <param name="consoleBuffer">A pointer to the ConsoleStreamBuffer where client output will be directed.</param>
	FRANAUDIO_API void RouteLoggingToConsole(FranAudioShared::Logger::ConsoleStreamBuffer* consoleBuffer);

	/// <summary>
	/// Checks if the current backend is valid for use.
	/// </summary>
	FRANAUDIO_API bool IsBackendValid();

	/// <summary>
	/// Sets the audio backend to use.
	/// </summary>
	/// <param name="type">The backend type to set, specified as a value of Backend::BackendType.</param>
	/// <returns>True if successful</returns>
	FRANAUDIO_API bool SetBackend(Backend::BackendType type);

	/// <summary>
	/// Get the current backend.
	/// 
	/// <para>
	/// NOTE: Do not cache the return of this function.
	/// It may change if the decoder is changed or destroyed.
	/// </para>
	/// 
	/// </summary>
	/// <returns>Pointer to the current backend</returns>
	FRANAUDIO_API Backend::Backend* GetBackend();
}
