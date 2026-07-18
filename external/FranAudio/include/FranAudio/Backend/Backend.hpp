// FranticDreamer 2022-2025
#pragma once

#include <string>
#include <memory>
#include <array>
#include <tuple>
#include <span>

#include "FranAudioAPI.hpp"

#include "Backend/BackendTypes.hpp"

#include "FranAudioShared/Containers/UnorderedMap.hpp"
#include "FranAudioShared/Containers/Vector.hpp"
#include "FranAudioShared/FranAudioShared.hpp"
#include "Decoder/Decoder.hpp"
#include "Sound/WaveData/WaveData.hpp"
#include "Sound/Sound.hpp"


/// <summary>
/// Namespace for Backend Abstract and Implementations.
/// </summary>
namespace FranAudio::Backend
{
	/// <summary>
	/// Abstract for backend implementations.
	/// </summary>
	class Backend
	{
	protected:
		/// <summary>
		/// A smart pointer to manage the current audio decoder instance.
		/// </summary>
		/// <remarks>
		/// There is a small design oversight problem.
		/// This was originally a raw pointer. So currentDecoder->Reset() was called so because there was no "confusion".
		/// But now it's a smart pointer, try not to call currentDecoder.reset() by mistake. IDE auto-complete may suggest it.
		/// </remarks>
		std::unique_ptr<FranAudio::Decoder::Decoder> currentDecoder = nullptr;
		FranAudio::Decoder::DecoderType currentDecoderType = FranAudio::Decoder::DecoderType::None;

		/// <summary>
		/// Next Sound ID to be used.
		/// This is used to generate unique IDs for sounds.
		/// </summary>
		size_t nextSoundID = 0;

		/// <summary>
		/// Next Wave Data ID to be used.
		/// Like sound IDs, wave data IDs are unique and never reused: unloading a file
		/// permanently invalidates its ID. Never reset, so stale IDs stay invalid.
		/// </summary>
		size_t nextWaveDataID = 0;

		/// <summary>
		/// Cache for decoded audio data, keyed by unique wave data ID.
		/// This is used to cache the decoded audio data to avoid decoding every time the audio is played.
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<size_t, FranAudio::Sound::WaveData> waveDataCache;

		/// <summary>
		/// Map for finding decoded audio data in the cache by canonical file path.
		/// This is what prevents the same file from being decoded twice.
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<std::string, size_t> filenameWaveMap;

		/// <summary>
		/// Canonicalise a path for use as a filenameWaveMap key: absolute, normalised,
		/// symlinks resolved, and lowercased on Windows. Different spellings of the same
		/// file ("a.wav", ".\a.wav", its absolute path) map to the same key.
		/// </summary>
		static std::string CanonicalisePath(const std::string& filename);

		/// <summary>
		/// Currently Active Sounds
		/// Tied to nextSoundID
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<size_t, FranAudio::Sound::Sound> activeSounds;

		// ========================
		// Sound Groups
		// ========================

		/// <summary>
		/// Volume multiplier per sound group. Groups are created implicitly on first use.
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<std::string, float> groupVolumes;

		/// <summary>
		/// Group assignment per sound ID. Sounds without an entry belong to no group.
		/// Stale entries of finished sounds are cleaned up lazily in SetGroupVolume.
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<size_t, std::string> soundGroups;

		/// <summary>
		/// The volume set by the user per sound ID, before the group multiplier is applied.
		/// The backend itself is given baseVolume * groupVolume.
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<size_t, float> soundBaseVolumes;

		/// <summary>
		/// Get the group volume multiplier that applies to a sound (1.0 if ungrouped).
		/// </summary>
		float GetGroupVolumeForSound(size_t soundID) const;

		/// <summary>
		/// Apply a volume directly to the backend's sound instance.
		/// This is the raw applied volume; group multipliers are handled by SetSoundVolume().
		/// </summary>
		/// <param name="soundID">ID of the sound to set the volume of</param>
		/// <param name="volume">Final volume to apply (0.0 - 1.0)</param>
		virtual void SetSoundVolumeRaw(size_t soundID, float volume) = 0;

		/// <summary>
		/// Read the volume currently applied to the backend's sound instance.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the volume of</param>
		/// <returns>Applied volume of the sound (0.0 - 1.0)</returns>
		virtual float GetSoundVolumeRaw(size_t soundID) = 0;

		// ========================
		// Decode Settings
		// ========================

		/// <summary>
		/// Default decode settings.
		/// </summary>
		FranAudio::Decoder::DecodeSettings defaultDecodeSettings = {};

		/// <summary>
		/// Settings for decoding audio wave data.
		/// </summary>
		FranAudio::Decoder::DecodeSettings currentDecodeSettings = {};

	public:
		//Backend() = default;
		FRANAUDIO_API virtual ~Backend();

		/// <summary>
		/// Initialise the backend.
		/// This is used to initialise the backend and set it up for use.
		/// Called by CreateBackend(). Unless you're manually managing backends, you shouldn't need to call this.
		/// 
		/// <para/> Note: The decoderType parameter is a hint, the backend may choose to ignore it and use its own default decoder.
		/// <para/> Note: Decoder is not initialised by this function, call SetDecoder() to initialise it if you're manually managing backends.
		/// </summary>
		/// <param name="decoderType">Type of the decoder to use. Default is DecoderType::None, which means the backend will choose the default decoder.</param>
		virtual FRANAUDIO_API bool Init(FranAudio::Decoder::DecoderType decoderType = FranAudio::Decoder::DecoderType::None);

		/// <summary>
		/// Reset the backend. 
		/// This is used to reset the backend to its initial state.
		/// 
		/// <para/> Note: This will stop all currently playing sounds and clear the active sounds list.
		/// <para/> Note: This will call Reset() on the decoder.
		/// </summary>
		virtual FRANAUDIO_API void Reset();

		/// <summary>
		/// Shutdown the backend.
		/// This is used to shut down the backend and clean up any resources.
		///
		/// <para/> Note: Called by DestroyBackend(). Unless you're manually managing backends, you shouldn't need to call this.
		/// <para/> Note: Unlike Init(), this function will also de-initialise the decoder if it was initialised since Decoder is managed by the backend after it's set.
		/// </summary>
		/// <param name="forReset">If true, the shutdown is for a reset operation. If false, it's a complete shutdown.</param>
		virtual FRANAUDIO_API void Shutdown(bool forReset = false);

		/// <summary>
		/// Get the backend type.
		/// </summary>
		/// <returns>Type of this Backend instance</returns>
		virtual constexpr FRANAUDIO_API BackendType GetBackendType() const noexcept
		{
			return BackendType::None;
		}

		/// <summary>
		/// Retrieves the name of the audio backend.
		/// </summary>
		/// <returns>Name of the backend.</returns>
		virtual constexpr FRANAUDIO_API const char* GetBackendName() const noexcept
		{
			return BackendTypeNames[static_cast<size_t>(GetBackendType())];
		}

		// ========================
		// Decoder Management
		// ========================

		// TODO: Should we make the supported decoders a bitmap or bool array?
		/// <summary>
		/// Get the supported decoders.
		/// </summary>
		/// <returns>List of supported decoders</returns>
		virtual const FranAudioShared::Containers::Vector<FranAudio::Decoder::DecoderType>& GetSupportedDecoders() const = 0;

		/// <summary>
		/// Get the decoder type.
		/// </summary>
		/// <returns>Type of the current decoder used by this backend</returns>
		virtual FRANAUDIO_API FranAudio::Decoder::DecoderType GetDecoderType() const;

		/// <summary>
		/// Get the name of the current decoder.
		/// </summary>
		/// <returns>Name of the current decoder used by this backend</returns>
		virtual constexpr FRANAUDIO_API const char* GetDecoderName() const noexcept
		{
			return currentDecoder->GetDecoderName();
		}

		/// <summary>
		/// Get the current decoder.
		/// 
		/// <para>
		/// NOTE: Do not cache the return of this function.
		/// It may change if the decoder is changed or destroyed.
		/// </para>
		/// 
		/// </summary>
		/// <returns>Pointer to the current decoder used by this backend</returns>
		FRANAUDIO_API FranAudio::Decoder::Decoder* GetCurrentDecoder() const;

		/// <summary>
		/// Set the decoder type.
		/// </summary>
		/// <param name="decoderType">Type of the decoder to replace the current one</param>
		/// <param name="force">Force the decoder to be replaced even if it's the same type</param>
		/// <returns>True if successful.</returns>
		FRANAUDIO_API bool SetDecoder(FranAudio::Decoder::DecoderType decoderType, bool force = false);

		/// <summary>
		/// Destroy the current decoder.
		/// </summary>
		FRANAUDIO_API void DestroyDecoder();

		/// <summary>
		/// Get the default decode settings.
		/// </summary>
		/// <returns>Default decode settings</returns>
		virtual const FRANAUDIO_API FranAudio::Decoder::DecodeSettings& GetDefaultDecodeSettings() const noexcept;

		/// <summary>
		/// Set the current decode settings.
		/// </summary>
		/// <param name="settings">New decode settings</param>
		virtual FRANAUDIO_API void SetDecodeSettings(const FranAudio::Decoder::DecodeSettings& settings) noexcept;

		/// <summary>
		/// Get the current decode settings.
		/// </summary>
		/// <returns>Current decode settings</returns>
		constexpr FRANAUDIO_API const FranAudio::Decoder::DecodeSettings& GetDecodeSettings() const noexcept
		{
			return currentDecodeSettings;
		}

		// ========================
		// Listener (3D Audio)
		// ========================

		/// <summary>
		/// Set the listener's position and orientation.
		/// </summary>
		/// <param name="position">New position of the listener</param>
		/// <param name="forward">New forward vector of the listener</param>
		/// <param name="up">New up vector of the listener</param>
		virtual void SetListenerTransform(const FranAudioShared::Vector3& position, const FranAudioShared::Vector3& forward, const FranAudioShared::Vector3& up) = 0;

		/// <summary>
		/// Set the listener's position and orientation using a struct.
		/// </summary>
		/// <param name="transform">New position and orientation of the listener</param>
		virtual FRANAUDIO_API void SetListenerTransform(const FranAudioShared::ListenerTransform& transform);

		/// <summary>
		/// Get the listener's position and orientation.
		/// </summary>
		/// <returns>Output position, forward and up vectors of the listener</returns>
		virtual FranAudioShared::ListenerTransform GetListenerTransform() = 0;

		/// <summary>
		/// Set the listener's position.
		/// </summary>
		/// <param name="position">New position of the listener</param> 
		virtual void SetListenerPosition(const FranAudioShared::Vector3& position) = 0;

		/// <summary>
		/// Get the listener's position.
		/// </summary>
		/// <returns>Output position of the listener</returns>
		virtual FranAudioShared::Vector3 GetListenerPosition() = 0;

		/// <summary>
		/// Set the listener's orientation.
		/// </summary>
		/// <param name="forward">New forward vector of the listener</param>
		/// <param name="up">New up vector of the listener</param>
		virtual void SetListenerOrientation(const FranAudioShared::Vector3& forward, const FranAudioShared::Vector3& up) = 0;

		/// <summary>
		/// Set the listener's orientation using a struct.
		/// </summary>
		/// <param name="orientation">New orientation of the listener</param>
		virtual FRANAUDIO_API void SetListenerOrientation(const FranAudioShared::ListenerOrientation& orientation);

		/// <summary>
		/// Get the listener's orientation.
		/// </summary>
		/// <returns>Output forward and up vectors of the listener</returns>
		virtual FranAudioShared::ListenerOrientation GetListenerOrientation() = 0;

		/// <summary>
		/// Set the master volume.
		/// Can also be the listener's hearing volume.
		/// </summary>
		/// <param name="volume">Volume to set the master volume to (0.0 - 1.0)</param>
		virtual void SetMasterVolume(float volume) = 0;

		/// <summary>
		/// Get the master volume.
		/// Can also be the listener's hearing volume.	
		/// </summary>
		virtual float GetMasterVolume() = 0; // Not const because some audio backends might require non-const pointer.

		// ========================
		// Audio File Management
		// ========================

		/// <summary>
		/// Play an audio file that is already loaded into memory.
		/// </summary>
		/// <param name="waveData">Wave data to play</param>
		/// <returns>Active Sounds List Index</returns>
		virtual size_t PlayAudioWave(const FranAudio::Sound::WaveData& waveData) = 0;

		/// <summary>
		/// Decode an audio file and load it into the memory.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <returns>Wave Data Cache Index</returns>
		virtual FRANAUDIO_API size_t LoadAudioFile(const std::string& filename);

		/// <summary>
		/// Decode an audio file and load it into the memory.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <param name="decodeSettings">Decode settings to use for this file. If not specified, current decode settings are used.</param>
		/// <returns>Wave Data Cache Index</returns>
		virtual FRANAUDIO_API size_t LoadAudioFile(const std::string& filename, const FranAudio::Decoder::DecodeSettings& decodeSettings);

		/// <summary>
		/// Play an audio file after checking if it's loaded.
		/// If the audio file is not loaded, it will not play and return SIZE_MAX.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <returns>Active Sounds List Index</returns>
		virtual FRANAUDIO_API size_t PlayAudioFile(const std::string& filename);

		/// <summary>
		/// Unload a previously loaded audio file and free its decoded data.
		/// Fails if any active sound is still playing this wave data.
		///
		/// <para/> Note: The freed cache slot is recycled by the next LoadAudioFile call,
		/// so wave data indices of other loaded files stay valid.
		/// </summary>
		/// <param name="filename">Path to the audio file that was loaded</param>
		/// <returns>True if the file was unloaded, false if it was not loaded or still in use</returns>
		virtual FRANAUDIO_API bool UnloadAudioFile(const std::string& filename);

		/// <summary>
		/// Play an audio file by streaming it from disk in chunks instead of decoding it
		/// fully into memory. Intended for music and other long files.
		///
		/// <para/> Note: Streamed sounds are not cached in the wave data cache and have no
		/// wave data index. All per-sound operations (volume, pitch, position, looping...)
		/// work as usual.
		/// <para/> Note: Backends without native streaming support fall back to a full
		/// in-memory decode via LoadAudioFile + PlayAudioFile.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <param name="looping">True to loop the sound, false to play it once</param>
		/// <returns>Active Sounds List Index</returns>
		virtual FRANAUDIO_API size_t PlayAudioFileStream(const std::string& filename, bool looping = false);

		/// <summary>
		/// Retrieves the currently loaded wave data, keyed by unique wave data ID.
		/// </summary>
		/// <returns>Wave data cache</returns>
		virtual const FRANAUDIO_API FranAudioShared::Containers::UnorderedMap<size_t, FranAudio::Sound::WaveData>& GetWaveDataCache();

		// ========================
		// Sound Management
		// ========================

		/// <summary>
		/// Check if a sound is valid by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound in to check</param>
		virtual FRANAUDIO_API bool IsSoundValid(size_t soundID);

		/// <summary>
		/// Stop and clear an active sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound in to stop</param>
		virtual void StopPlayingSound(size_t soundID) = 0;

		/// <summary>
		/// Set whether the sound is paused or not by its ID.
		/// </summary>
		/// <param name="soundID">ID of the sound to modify</param>
		/// <param name="isPaused">True to pause the sound, false to resume playback</param>
		virtual void SetSoundPaused(size_t soundID, bool isPaused) = 0;

		/// <summary>
		/// Check if a sound is paused or not by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to check</param>
		/// <returns>True if the sound is paused, false if not</returns>
		virtual bool IsSoundPaused(size_t soundID) = 0;

		/// <summary>
		/// Set the volume of a playing sound by its index.
		/// If the sound is in a group, the group's volume multiplier is applied on top.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the volume of</param>
		/// <param name="volume">Volume to set the sound to (0.0 - 1.0)</param>
		FRANAUDIO_API void SetSoundVolume(size_t soundID, float volume);

		/// <summary>
		/// Get the volume of a playing sound by its index.
		/// This is the volume set by SetSoundVolume, without the group multiplier.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the volume of</param>
		/// <returns>Volume of the sound (0.0 - 1.0)</returns>
		FRANAUDIO_API float GetSoundVolume(size_t soundID);

		// ========================
		// Sound Groups
		// ========================

		/// <summary>
		/// Assign a sound to a group (e.g. "sfx", "music", "voice").
		/// Groups are created implicitly; the group's volume multiplier is applied immediately.
		/// Group names must not contain the '|' character (reserved by the network protocol).
		/// </summary>
		/// <param name="soundID">ID of the sound to assign</param>
		/// <param name="groupName">Name of the group</param>
		FRANAUDIO_API void SetSoundGroup(size_t soundID, const std::string& groupName);

		/// <summary>
		/// Get the group a sound belongs to.
		/// Every sound belongs to a group; unassigned sounds are in the default
		/// "__ungrpd__" group (FranAudioShared::defaultSoundGroupName).
		/// </summary>
		/// <param name="soundID">ID of the sound to check</param>
		/// <returns>Group name</returns>
		FRANAUDIO_API std::string GetSoundGroup(size_t soundID) const;

		/// <summary>
		/// Set the volume multiplier of a group and reapply it to all sounds in the group.
		/// </summary>
		/// <param name="groupName">Name of the group</param>
		/// <param name="volume">Volume multiplier for the group (0.0 - 1.0)</param>
		FRANAUDIO_API void SetGroupVolume(const std::string& groupName, float volume);

		/// <summary>
		/// Get the volume multiplier of a group.
		/// </summary>
		/// <param name="groupName">Name of the group</param>
		/// <returns>Volume multiplier of the group (1.0 if never set)</returns>
		FRANAUDIO_API float GetGroupVolume(const std::string& groupName) const;

		/// <summary>
		/// Set the pitch of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the pitch of</param>
		/// <param name="pitch">Pitch to set the sound to (1.0 = normal pitch)</param>
		virtual void SetSoundPitch(size_t soundID, float pitch) = 0;

		/// <summary>
		/// Get the pitch of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the pitch of</param>
		/// <returns>Pitch of the sound (1.0 = normal pitch)</returns>
		virtual float GetSoundPitch(size_t soundID) = 0;

		/// <summary>
		/// Set whether a playing sound loops by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to modify</param>
		/// <param name="looping">True to loop the sound, false to play it once</param>
		virtual void SetSoundLooping(size_t soundID, bool looping) = 0;

		/// <summary>
		/// Check if a playing sound loops by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to check</param>
		/// <returns>True if the sound is looping, false if not</returns>
		virtual bool IsSoundLooping(size_t soundID) = 0;

		/// <summary>
		/// Set the position of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the position of</param>
		/// <param name="position">Position to set the sound to</param>
		virtual void SetSoundPosition(size_t soundID, const FranAudioShared::Vector3& position) = 0;

		/// <summary>
		/// Get the position of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the position of</param>
		/// <returns>Position of the sound</returns>
		virtual FranAudioShared::Vector3 GetSoundPosition(size_t soundID) = 0;

		/// <summary>
		/// Set the attenuation parameters of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the attenuation of</param>
		/// <param name="rolloffFactor">Rolloff factor to set the sound attenuation to</param>
		/// <param name="minDistance">Minimum attenuation distance to set the sound attenuation to</param>
		/// <param name="maxDistance">Maximum attenuation distance to set the sound attenuation to</param>
		virtual void SetSoundAttenuation(size_t soundID, float rolloffFactor, float minDistance, float maxDistance) = 0;

		/// <summary>
		/// Set the attenuation parameters of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the attenuation of</param>
		/// <param name="attenuation">Attenuation parameters to set the new sound attenuation to</param>
		virtual void SetSoundAttenuation(size_t soundID, const FranAudioShared::SoundAttenuation& attenuation)
		{
			SetSoundAttenuation(soundID, attenuation.rolloffFactor, attenuation.minDistance, attenuation.maxDistance);
		}

		/// <summary>
		/// Get the attenuation parameters of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the attenuation of</param>
		/// <returns>Attenuation parameters of the sound</returns>
		virtual FranAudioShared::SoundAttenuation GetSoundAttenuation(size_t soundID) = 0;

		/// <summary>
		/// Get a reference to a playing sound by its index.
		/// Index MUST be valid. Check with IsSoundValid() first.
		/// </summary>
		/// <param name="soundID">ID of the sound to get</param>
		virtual FRANAUDIO_API Sound::Sound& GetSound(size_t soundID);
		
		/// <summary>
		/// Get the map of currently active sounds.
		/// </summary>
		/// <returns>Map of currently active sounds</returns>
		virtual const FRANAUDIO_API FranAudioShared::Containers::UnorderedMap<size_t, Sound::Sound>& GetActiveSounds() const;

		/// <summary>
		/// Retrieves a list of active sound IDs.
		/// </summary>
		/// <returns>A vector containing the IDs of currently active sounds.</returns>
		virtual const FRANAUDIO_API FranAudioShared::Containers::Vector<size_t> GetActiveSoundIDs() const;

		// ========================
		// Backend
		// ========================

		/// <summary>
		/// Create a backend instance.
		/// </summary>
		/// <param name="backendType">Type of the backend to create</param>
		/// <returns>Pointer to the created backend instance</returns>
		static FRANAUDIO_API std::unique_ptr<FranAudio::Backend::Backend> CreateBackend(BackendType backendType);
	};
}
