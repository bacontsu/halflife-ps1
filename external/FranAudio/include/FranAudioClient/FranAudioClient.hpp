// FranticDreamer 2022-2025
#pragma once

#include "FranAudioClientAPI.hpp"
#include "FranAudioShared/Logger/Logger.hpp"

#include "Decoder/DecodeSettings.hpp"
#include "Decoder/DecoderTypes.hpp"
#include "Backend/BackendTypes.hpp"
#include "Sound/WaveData/WaveFormats.hpp"

namespace FranAudioShared::Network
{
	struct NetworkFunction;
}

/// <summary>
/// Client API for FranAudioServer IPC system.
/// </summary>
namespace FranAudioClient
{
	// ===========================
	// Functions with Platform Specific Implementations
	// ===========================

	/// <summary>
	/// Initialise the FranAudio Client.
	/// This must be called before any other functions are used.
	/// 
	/// <para/> NOTE: This should be called after the server is started.
	/// </summary>
	FRANAUDIO_CLIENT_API void Init(bool isTestmode = false);

	/// <summary>
	/// Shutdown the FranAudio Client.
	/// This is for cleaning up resources used by the client.
	/// </summary>
	FRANAUDIO_CLIENT_API void Shutdown();

	/// <summary>
	/// Attempt to reconnect to the server.
	/// </summary>
	/// <returns>True if reconnection was successful, false otherwise</returns>
	FRANAUDIO_CLIENT_API bool Reconnect();

	/// <summary>
	/// Send a message to the server, and wait for a response.
	/// Response will be returned as a string, and will be empty if no response is received.
	///
	/// - Message format should be like this:
	/// "$[functionName]|[param1]|[param2]|...|[paramN]"
	/// </summary>
	/// 
	/// <param name="message">Message to Send</param>
	/// <returns>Response from the server, empty if no response is received</returns>
	FRANAUDIO_CLIENT_API std::string Send(std::string message);

	/// <summary>
	/// Send a message to the server, and wait for a response.
	/// Response will be returned as a string, and will be empty if no response is received.
	///
	/// <param name="message">Message to Send</param>
	/// <returns>Response from the server, empty if no response is received</returns>
	FRANAUDIO_CLIENT_API std::string Send(const FranAudioShared::Network::NetworkFunction& message);

	/// <summary>
	/// Send a fire-and-forget message to the server without waiting for a response.
	/// The server executes the command but does not reply, so there is no round-trip cost.
	/// Use for setters; errors are only visible in the server log.
	/// </summary>
	/// <param name="message">Message to Send</param>
	FRANAUDIO_CLIENT_API void SendNoReply(const FranAudioShared::Network::NetworkFunction& message);

	// ===========================
	// End of Functions with Platform Specific Implementations
	// ===========================

	// ========================
	// Shared Logger Usage
	// ========================

	/// <summary>
	/// Routes the client output to the specified console stream buffer.
	/// </summary>
	/// <param name="consoleBuffer">A pointer to the ConsoleStreamBuffer where client output will be directed.</param>
	FRANAUDIO_CLIENT_API void RouteClientLoggingToConsole(FranAudioShared::Logger::ConsoleStreamBuffer* consoleBuffer);

	/// <summary>
	/// FranAudio client wrapper functions.
	/// </summary>
	namespace Wrapper
	{
		/// <summary>
		/// Clears the client-side cache of last-known values (master volume, listener
		/// transform, per-sound state, group volumes). Called automatically by Init and
		/// Reconnect; call manually if the server was restarted behind the client's back.
		/// </summary>
		FRANAUDIO_CLIENT_API void ClearCache();

		/// <summary>
		/// Sets the audio backend to use.
		/// </summary>
		/// <param name="backendType">The backend type to set, specified as a value of Backend::BackendType.</param>
		/// <returns>True if successful.</returns>
		FRANAUDIO_CLIENT_API bool SetBackend(FranAudio::Backend::BackendType backendType);

		/// <summary>
		/// Backend related functions of the wrapper.
		/// </summary>
		namespace Backend
		{
			/// <summary>
			/// Get the backend type.
			/// </summary>
			/// <returns>Type of this Backend instance</returns>
			FRANAUDIO_CLIENT_API FranAudio::Backend::BackendType GetBackendType();

			/// <summary>
			/// Retrieves the name of the audio backend.
			/// </summary>
			/// <returns>Name of the backend.</returns>
			FRANAUDIO_CLIENT_API std::string GetBackendName();

			// ========================
			// Decoder Management
			// ========================
			
			/// <summary>
			/// Set the decoder type.
			/// </summary>
			/// <param name="decoderType">Type of the decoder to replace the current one</param>
			/// <returns>True if successful.</returns>
			FRANAUDIO_CLIENT_API bool SetDecoder(FranAudio::Decoder::DecoderType decoderType);

			/// <summary>
			/// Get the decoder type.
			/// </summary>
			/// <returns>Type of the current decoder used by this backend</returns>
			FRANAUDIO_CLIENT_API FranAudio::Decoder::DecoderType GetDecoderType();

			/// <summary>
			/// Get the name of the current decoder.
			/// </summary>
			/// <returns>Name of the current decoder used by this backend</returns>
			FRANAUDIO_CLIENT_API std::string GetDecoderName();

			/// <summary>
			/// Get the default decode settings.
			/// </summary>
			/// <returns>Default decode settings</returns>
			FRANAUDIO_CLIENT_API const FranAudio::Decoder::DecodeSettings GetDefaultDecodeSettings();

			/// <summary>
			/// Set the current decode settings.
			/// </summary>
			/// <param name="settings">New decode settings</param>
			FRANAUDIO_CLIENT_API void SetDecodeSettings(const FranAudio::Decoder::DecodeSettings& settings);

			/// <summary>
			/// Get the current decode settings.
			/// </summary>
			/// <returns>Current decode settings</returns>
			FRANAUDIO_CLIENT_API const FranAudio::Decoder::DecodeSettings GetDecodeSettings();

			// ========================
			// Listener (3D Audio)
			// ========================
	
			/// <summary>
			/// Set the listener's position and orientation.
			/// </summary>
			/// <param name="position">New position of the listener</param>
			/// <param name="forward">New forward vector of the listener</param>
			/// <param name="up">New up vector of the listener</param>
			FRANAUDIO_CLIENT_API void SetListenerTransform(float position[3], float forward[3], float up[3]);

			/// <summary>
			/// Get the listener's position and orientation.
			/// </summary>
			/// <param name="outPosition">Output position of the listener</param>
			/// <param name="outForward">Output forward vector of the listener</param>
			/// <param name="outUp">Output up vector of the listener</param>
			FRANAUDIO_CLIENT_API void GetListenerTransform(float outPosition[3], float outForward[3], float outUp[3]);

			/// <summary>
			/// Set the listener's position.
			/// </summary>
			/// <param name="position">New position of the listener</param> 
			FRANAUDIO_CLIENT_API void SetListenerPosition(const float position[3]);

			/// <summary>
			/// Get the listener's position.
			/// </summary>
			/// <param name="outPosition">Output position of the listener</param>
 			FRANAUDIO_CLIENT_API void GetListenerPosition(float outPosition[3]);

			/// <summary>
			/// Set the listener's orientation.
			/// </summary>
			/// <param name="forward">New forward vector of the listener</param>
			/// <param name="up">New up vector of the listener</param>
			FRANAUDIO_CLIENT_API void SetListenerOrientation(const float forward[3], const float up[3]);

			/// <summary>
			/// Get the listener's orientation.
			/// </summary>
			/// <param name="outForward">Output forward vector of the listener</param>
			/// <param name="outUp">Output up vector of the listener</param>
			FRANAUDIO_CLIENT_API void GetListenerOrientation(float outForward[3], float outUp[3]);

			/// <summary>
			/// Set the master volume.
			/// Can also be the listener's hearing volume.
			/// </summary>
			/// <param name="volume">Volume to set the master volume to (0.0 - 1.0)</param>
			FRANAUDIO_CLIENT_API void SetMasterVolume(float volume);

			/// <summary>
			/// Get the master volume.
			/// Can also be the listener's hearing volume.	
			/// </summary>
			FRANAUDIO_CLIENT_API float GetMasterVolume();

			// ========================
			// Audio File Management
			// ========================

			/// <summary>
			/// Decode an audio file and load it into the memory.
			/// </summary>
			/// <param name="filename">Path to the audio file</param>
			/// <returns>Wave Data Cache Index</returns>
			FRANAUDIO_CLIENT_API size_t LoadAudioFile(const std::string& filename);

			/// <summary>
			/// Decode an audio file and load it into the memory.
			/// </summary>
			/// <param name="filename">Path to the audio file</param>
			/// <param name="decodeSettings">Decode settings to use for this file. If not specified, current decode settings are used.</param>
			/// <returns>Wave Data Cache Index</returns>
			FRANAUDIO_CLIENT_API size_t LoadAudioFile(const std::string& filename, const FranAudio::Decoder::DecodeSettings& decodeSettings);

			/// <summary>
			/// Unload a previously loaded audio file and free its decoded data on the server.
			/// Fails if any active sound is still playing this wave data.
			/// </summary>
			/// <param name="filename">Path to the audio file that was loaded</param>
			/// <returns>True if the file was unloaded, false if it was not loaded or still in use</returns>
			FRANAUDIO_CLIENT_API bool UnloadAudioFile(const std::string& filename);

			/// <summary>
			/// Play an audio file after checking if it's loaded.
			/// If the audio file is not loaded, it will be loaded and then played.
			/// </summary>
			/// <param name="filename">Path to the audio file</param>
			/// <param name="looping">True to loop the sound, false to play it once (default)</param>
			/// <returns>Active Sounds List Index</returns>
			FRANAUDIO_CLIENT_API size_t PlayAudioFile(const std::string& filename, bool looping = false);

			/// <summary>
			/// Play an audio file by streaming it from disk in chunks instead of decoding it
			/// fully into memory. Intended for music and other long files.
			/// </summary>
			/// <param name="filename">Path to the audio file</param>
			/// <param name="looping">True to loop the sound, false to play it once (default)</param>
			/// <returns>Active Sounds List Index</returns>
			FRANAUDIO_CLIENT_API size_t PlayAudioFileStream(const std::string& filename, bool looping = false);

			/// <summary>
			/// Get the list of audio files currently loaded on the server.
			/// </summary>
			/// <returns>A vector of (wave data ID, filename) pairs</returns>
			FRANAUDIO_CLIENT_API const FranAudioShared::Containers::Vector<std::pair<size_t, std::string>> GetLoadedAudioFiles();

			/// <summary>
			/// Set the volume multiplier of a sound group (e.g. "sfx", "music", "voice").
			/// Applied on top of each grouped sound's own volume.
			/// </summary>
			/// <param name="groupName">Name of the group (must not contain '|')</param>
			/// <param name="volume">Volume multiplier for the group (0.0 - 1.0)</param>
			FRANAUDIO_CLIENT_API void SetGroupVolume(const std::string& groupName, float volume);

			/// <summary>
			/// Get the volume multiplier of a sound group.
			/// </summary>
			/// <param name="groupName">Name of the group</param>
			/// <returns>Volume multiplier of the group (1.0 if never set)</returns>
			FRANAUDIO_CLIENT_API float GetGroupVolume(const std::string& groupName);

			// ========================
			// Macro Sound Management
			// ========================

			/// <summary>
			/// Retrieves a list of active sound IDs.
			/// </summary>
			/// <returns>A vector containing the IDs of currently active sounds.</returns>
			FRANAUDIO_CLIENT_API const FranAudioShared::Containers::Vector<size_t> GetActiveSoundIDs();
		}

		/// <summary>
		/// Sound related functions of the wrapper.
		/// </summary>
		namespace Sound
		{
			// ========================
			// Sound Management
			// ========================

			/// <summary>
			/// Check if a sound is valid by its index.
			/// </summary>
			/// <param name="soundIndex">Index of the sound in the active sounds list</param>
			FRANAUDIO_CLIENT_API bool IsValid(size_t soundIndex);

			/// <summary>
			/// Stop and clear an active sound by its index.
			/// </summary>
			/// <param name="soundIndex">Index of the sound in the active sounds list</param>
			FRANAUDIO_CLIENT_API void Stop(size_t soundIndex);

			/// <summary>
			/// Set whether the sound is paused or not by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to modify</param>
			/// <param name="isPaused">True to pause the sound, false to resume playback</param>
			FRANAUDIO_CLIENT_API void SetPaused(size_t soundID, bool isPaused);

			/// <summary>
			/// Check if a sound is paused or not by its index.
			/// </summary>
			/// <param name="soundID">ID of the sound to check</param>
			/// <returns>True if the sound is paused, false if not</returns>
			FRANAUDIO_CLIENT_API bool IsPaused(size_t soundID);

			/// <summary>
			/// Set the volume of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to set the volume of</param>
			/// <param name="volume">Volume to set the sound to (0.0 - 1.0)</param>
			FRANAUDIO_CLIENT_API void SetVolume(size_t soundID, float volume);

			/// <summary>
			/// Get the volume of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to get the volume of</param>
			/// <returns>Volume of the sound (0.0 - 1.0)</returns>
			FRANAUDIO_CLIENT_API float GetVolume(size_t soundID);

			/// <summary>
			/// Set the pitch of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to set the pitch of</param>
			/// <param name="pitch">Pitch to set the sound to (1.0 = normal pitch)</param>
			FRANAUDIO_CLIENT_API void SetPitch(size_t soundID, float pitch);

			/// <summary>
			/// Get the pitch of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to get the pitch of</param>
			/// <returns>Pitch of the sound (1.0 = normal pitch)</returns>
			FRANAUDIO_CLIENT_API float GetPitch(size_t soundID);

			/// <summary>
			/// Set whether a playing sound loops by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to modify</param>
			/// <param name="looping">True to loop the sound, false to play it once</param>
			FRANAUDIO_CLIENT_API void SetLooping(size_t soundID, bool looping);

			/// <summary>
			/// Check if a playing sound loops by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to check</param>
			/// <returns>True if the sound is looping, false if not</returns>
			FRANAUDIO_CLIENT_API bool IsLooping(size_t soundID);

			/// <summary>
			/// Assign a playing sound to a group (e.g. "sfx", "music", "voice").
			/// </summary>
			/// <param name="soundID">ID of the sound to assign</param>
			/// <param name="groupName">Name of the group (must not contain '|')</param>
			FRANAUDIO_CLIENT_API void SetGroup(size_t soundID, const std::string& groupName);

			/// <summary>
			/// Get the group a playing sound belongs to.
			/// Every sound belongs to a group.
			/// Unassigned sounds are in the default "__ungrpd__" group (FranAudioShared::defaultSoundGroupName).
			/// </summary>
			/// <param name="soundID">ID of the sound to check</param>
			/// <returns>Group name</returns>
			FRANAUDIO_CLIENT_API std::string GetGroup(size_t soundID);

			/// <summary>
			/// Set the position of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to set the position of</param>
			/// <param name="position">Position to set the sound to</param>
			FRANAUDIO_CLIENT_API void SetPosition(size_t soundID, float position[3]);

			/// <summary>
			/// Get the position of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to get the position of</param>
			/// <param name="position">Output position of the sound</param>
			FRANAUDIO_CLIENT_API void GetPosition(size_t soundID, float position[3]);

			/// <summary>
			/// Set the attenuation parameters of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to set the attenuation of</param>
			/// <param name="rolloffFactor">Rolloff factor for the sound</param>
			/// <param name="minDistance">Minimum distance for the sound</param>
			/// <param name="maxDistance">Maximum distance for the sound</param>
			FRANAUDIO_CLIENT_API void SetAttenuation(size_t soundID, float rolloffFactor, float minDistance, float maxDistance);

			/// <summary>
			/// Get the attenuation parameters of a playing sound by its ID.
			/// </summary>
			/// <param name="soundID">ID of the sound to get the attenuation of</param>
			/// <param name="outRolloffFactor">Output rolloff factor of the sound</param>
			/// <param name="outMinDistance">Output minimum distance of the sound</param>
			/// <param name="outMaxDistance">Output maximum distance of the sound</param>
			FRANAUDIO_CLIENT_API void GetAttenuation(size_t soundID, float& outRolloffFactor, float& outMinDistance, float& outMaxDistance);
		}
	}
}
