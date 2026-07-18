// FranticDreamer 2022-2025
#pragma once

//#ifdef FRANAUDIO_USE_VORBIS
#include "miniaudio/extras/decoders/libvorbis/miniaudio_libvorbis.h"
//#endif
//#ifdef FRANAUDIO_USE_OPUS
#include "miniaudio/extras/decoders/libopus/miniaudio_libopus.h"
//#endif

#include "Backend/Backend.hpp"
#include "Sound/Sound.hpp"
#include "Sound/WaveData/WaveData.hpp"

namespace FranAudio::Backend
{
	inline ma_decoding_backend_vtable* miniaudio_backendVTables[] =
	{
	#ifdef FRANAUDIO_USE_VORBIS
		ma_decoding_backend_libvorbis,
	#endif
	#ifdef FRANAUDIO_USE_OPUS
		ma_decoding_backend_libopus,
	#endif
	// If neither vorbis nor opus are defined, this will be nullptr.
	#if !defined(FRANAUDIO_USE_VORBIS) && !defined(FRANAUDIO_USE_OPUS)
		nullptr,
	#endif
	};

	/// <summary>
	/// Miniaudio Backend
	/// </summary>
	class miniaudio : public Backend
	{
	private:
		ma_engine engine = {};
		ma_engine_config engineConfig = {};
		ma_device device = {};
		ma_device_config deviceConfig = {};
		ma_decoder_config defaultDecoderConfig = {};

		/// <summary>
		/// True while the engine and device are initialised.
		/// Guards against double-uninitialisation.
		/// </summary>
		bool isEngineInitialised = false;

		// ==========
		// VORBIS
		// ==========

		ma_libvorbis libvorbis = {};
		
		// ==========
		// OPUS
		// ==========

		//ma_libopus libopus = {};


		/// <summary>
		/// Sound data in a format that can be played by the miniaudio backend.
		/// </summary>
		struct MiniaudioSound
		{
			/// <summary>
			/// Miniaudio audio buffer config.
			/// </summary>
			ma_audio_buffer_config audioBufferConfig = {};

			/// <summary>
			/// Miniaudio audio buffer.
			/// </summary>
			ma_audio_buffer audioBuffer = {};

			/// <summary>
			/// The actual miniaudio sound object.
			/// </summary>
			ma_sound sound = {};

			/// <summary>
			/// True if the sound streams from disk; it has no audio buffer to uninitialise.
			/// </summary>
			bool isStreamed = false;

			/// <summary>
			/// Whether the sound is paused or not.
			/// </summary>
			bool isPaused = false;

			/// <summary>
			/// Time the sound was paused at in milliseconds.
			/// </summary>
			size_t pausedTime = 0;
		};

		/// <summary>
		/// A map of active sounds' corresponding data in a format that miniaudio can play.
		/// 
		/// This is used for making miniaudio interaction easier.
		/// </summary>
		FranAudioShared::Containers::UnorderedMap<size_t, std::unique_ptr<MiniaudioSound>> miniaudioSoundData;

	public:
		/// <summary>
		/// Safety Net: Uninitialises the engine and device if Shutdown() was never called.
		/// So destroying a live backend can't cause a segfault.
		/// </summary>
		FRANAUDIO_API ~miniaudio() override;

		/// <summary>
		/// Initialise the backend.
		/// This is used to initialise the backend and set it up for use.
		/// Called by CreateBackend(). Unless you're manually managing backends, you shouldn't need to call this.
		/// 
		/// <para/> Note: The decoderType parameter is a hint, the backend may choose to ignore it and use its own default decoder.
		/// <para/> Note: Decoder is not initialised by this function, call SetDecoder() to initialise it if you're manually managing backends.
		/// </summary>
		/// <param name="decoderType">Type of the decoder to use. Default is DecoderType::None, which means the backend will choose the default decoder.</param>
		virtual FRANAUDIO_API bool Init(FranAudio::Decoder::DecoderType decoderType = FranAudio::Decoder::DecoderType::None) override;

		/// <summary>
		/// Reset the backend. 
		/// This is used to reset the backend to its initial state.
		/// 
		/// <para/> Note: This will stop all currently playing sounds and clear the active sounds list.
		/// <para/> Note: This will call Reset() on the decoder.
		/// </summary>
		virtual FRANAUDIO_API void Reset() override;

		/// <summary>
		/// Shutdown the backend.
		/// This is used to shut down the backend and clean up any resources.
		///
		/// <para/> Note: Called by DestroyBackend(). Unless you're manually managing backends, you shouldn't need to call this.
		/// <para/> Note: Unlike Init(), this function will also de-initialise the decoder if it was initialised since Decoder is managed by the backend after it's set.
		/// </summary>
		/// <param name="forReset">If true, the shutdown is for a reset operation. If false, it's a complete shutdown.</param>
		virtual FRANAUDIO_API void Shutdown(bool forReset = false) override;

		/// <summary>
		/// Get the backend type.
		/// </summary>
		/// <returns>Type of this Backend instance</returns>
		virtual constexpr FRANAUDIO_API BackendType GetBackendType() const noexcept override
		{
			return BackendType::miniaudio;
		}

		// ========================
		// Decoder Management
		// ========================

		/// <summary>
		/// Get the supported decoders.
		/// </summary>
		/// <returns>List of supported decoders</returns>
		virtual const FRANAUDIO_API FranAudioShared::Containers::Vector<FranAudio::Decoder::DecoderType>& GetSupportedDecoders() const override;

		// ========================
		// Listener (3D Audio)
		// ========================

		/// <summary>
		/// Set the listener's position and orientation.
		/// </summary>
		/// <param name="position">New position of the listener</param>
		/// <param name="forward">New forward vector of the listener</param>
		/// <param name="up">New up vector of the listener</param>
		virtual FRANAUDIO_API void SetListenerTransform(const FranAudioShared::Vector3& position, const FranAudioShared::Vector3& forward, const FranAudioShared::Vector3& up) override;

		/// <summary>
		/// Get the listener's position and orientation.
		/// </summary>
		/// <returns>Output position, forward and up vectors of the listener</returns>
		virtual FRANAUDIO_API FranAudioShared::ListenerTransform GetListenerTransform() override;

		/// <summary>
		/// Set the listener's position.
		/// </summary>
		/// <param name="position">New position of the listener</param>
		virtual FRANAUDIO_API void SetListenerPosition(const FranAudioShared::Vector3& position) override;

		/// <summary>
		/// Get the listener's position.
		/// </summary>
		/// <returns>Output position of the listener</returns>
		virtual FRANAUDIO_API FranAudioShared::Vector3 GetListenerPosition() override;

		/// <summary>
		/// Set the listener's orientation.
		/// </summary>
		/// <param name="forward">New forward vector of the listener</param>
		/// <param name="up">New up vector of the listener</param>
		virtual FRANAUDIO_API void SetListenerOrientation(const FranAudioShared::Vector3& forward, const FranAudioShared::Vector3& up) override;

		/// <summary>
		/// Get the listener's orientation.
		/// </summary>
		/// <returns>Output forward and up vectors of the listener</returns>
		virtual FRANAUDIO_API FranAudioShared::ListenerOrientation GetListenerOrientation() override;

		/// <summary>
		/// Set the master volume.
		/// Can also be the listener's hearing volume.
		/// </summary>
		/// <param name="volume">Volume to set the master volume to (0.0 - 1.0)</param>
		virtual FRANAUDIO_API void SetMasterVolume(float volume) override;

		/// <summary>
		/// Get the master volume.
		/// Can also be the listener's hearing volume.
		/// </summary>
		virtual FRANAUDIO_API float GetMasterVolume() override; // Not const because some audio backends might require non-const pointer.

		// ========================
		// Audio File Management
		// ========================

		/// <summary>
		/// Play an audio file that is already loaded into memory.
		/// </summary>
		/// <param name="waveData">Wave data to play</param>
		/// <returns>Active Sounds List Index</returns>
		virtual FRANAUDIO_API size_t PlayAudioWave(const FranAudio::Sound::WaveData& waveData) override;

		/// <summary>
		/// Decode an audio file and load it into the memory.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <returns>Wave Data Cache Index</returns>
		virtual FRANAUDIO_API size_t LoadAudioFile(const std::string& filename) override;

		/// <summary>
		/// Decode an audio file and load it into the memory.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <param name="decodeSettings">Decode settings to use for this file. If not specified, current decode settings are used.</param>
		/// <returns>Wave Data Cache Index</returns>
		virtual FRANAUDIO_API size_t LoadAudioFile(const std::string& filename, const FranAudio::Decoder::DecodeSettings& decodeSettings) override;

		/// <summary>
		/// Play an audio file after checking if it's loaded.
		/// If the audio file is not loaded, it will be loaded and then played.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <returns>Active Sounds List Index</returns>
		virtual FRANAUDIO_API size_t PlayAudioFile(const std::string& filename) override;

		/// <summary>
		/// Play an audio file by streaming it from disk in chunks (native miniaudio streaming).
		/// Intended for music and other long files; nothing is added to the wave data cache.
		/// </summary>
		/// <param name="filename">Path to the audio file</param>
		/// <param name="looping">True to loop the sound, false to play it once</param>
		/// <returns>Active Sounds List Index</returns>
		virtual FRANAUDIO_API size_t PlayAudioFileStream(const std::string& filename, bool looping = false) override;

		// ========================
		// Sound Management
		// ========================

		/// <summary>
		/// Check if a sound is valid by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound in to check</param>
		virtual FRANAUDIO_API bool IsSoundValid(size_t soundID) override;

		/// <summary>
		/// Stop and clear an active sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound in to stop</param>
		virtual FRANAUDIO_API void StopPlayingSound(size_t soundID) override;

		/// <summary>
		/// Set whether the sound is paused or not by its ID.
		/// </summary>
		/// <param name="soundID">ID of the sound to modify</param>
		/// <param name="isPaused">True to pause the sound, false to resume playback</param>
		virtual FRANAUDIO_API void SetSoundPaused(size_t soundID, bool isPaused) override;
		
		/// <summary>
		/// Check if a sound is paused or not by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to check</param>
		/// <returns>True if the sound is paused, false if not</returns>
		virtual FRANAUDIO_API bool IsSoundPaused(size_t soundID) override;

		/// <summary>
		/// Apply a volume directly to the miniaudio sound instance.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the volume of</param>
		/// <param name="volume">Final volume to apply (0.0 - 1.0)</param>
		virtual FRANAUDIO_API void SetSoundVolumeRaw(size_t soundID, float volume) override;

		/// <summary>
		/// Read the volume currently applied to the miniaudio sound instance.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the volume of</param>
		/// <returns>Applied volume of the sound (0.0 - 1.0)</returns>
		virtual FRANAUDIO_API float GetSoundVolumeRaw(size_t soundID) override;

		/// <summary>
		/// Set the pitch of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the pitch of</param>
		/// <param name="pitch">Pitch to set the sound to (1.0 = normal pitch)</param>
		virtual FRANAUDIO_API void SetSoundPitch(size_t soundID, float pitch) override;

		/// <summary>
		/// Get the pitch of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the pitch of</param>
		/// <returns>Pitch of the sound (1.0 = normal pitch)</returns>
		virtual FRANAUDIO_API float GetSoundPitch(size_t soundID) override;

		/// <summary>
		/// Set whether a playing sound loops by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to modify</param>
		/// <param name="looping">True to loop the sound, false to play it once</param>
		virtual FRANAUDIO_API void SetSoundLooping(size_t soundID, bool looping) override;

		/// <summary>
		/// Check if a playing sound loops by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to check</param>
		/// <returns>True if the sound is looping, false if not</returns>
		virtual FRANAUDIO_API bool IsSoundLooping(size_t soundID) override;

		/// <summary>
		/// Set the position of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the position of</param>
		/// <param name="position">New position of the sound</param>
		virtual FRANAUDIO_API void SetSoundPosition(size_t soundID, const FranAudioShared::Vector3& position) override;

		/// <summary>
		/// Get the position of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the position of</param>
		/// <returns>Position of the sound</returns>
		virtual FRANAUDIO_API FranAudioShared::Vector3 GetSoundPosition(size_t soundID) override;

		/// <summary>
		/// Set the attenuation parameters of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to set the attenuation of</param>
		/// <param name="rolloffFactor">Rolloff factor to set the sound attenuation to</param>
		/// <param name="minDistance">Minimum attenuation distance to set the sound attenuation to</param>
		/// <param name="maxDistance">Maximum attenuation distance to set the sound attenuation to</param>
		virtual FRANAUDIO_API void SetSoundAttenuation(size_t soundID, float rolloffFactor, float minDistance, float maxDistance) override;

		/// <summary>
		/// Get the attenuation parameters of a playing sound by its index.
		/// </summary>
		/// <param name="soundID">ID of the sound to get the attenuation of</param>
		/// <returns>Attenuation parameters of the sound</returns>
		virtual FRANAUDIO_API FranAudioShared::SoundAttenuation GetSoundAttenuation(size_t soundID) override;

		// ========================
		// Miniaudio Specific
		// ========================

		/// <summary>
		/// Get the default decoder configuration.
		/// </summary>
		/// <returns>Default decoder configuration</returns>
		ma_decoder_config* GetDefaultDecoderConfig();

		/// <summary>
		/// Convert a miniaudio format to a FranAudio format.
		/// </summary>
		/// <param name="format">miniaudio format</param>
		/// <returns>FranAudio format</returns>
		static Sound::WaveFormat ConvertFormat(ma_format format);

		/// <summary>
		/// Convert a FranAudio format to a miniaudio format.
		/// </summary>
		/// <param name="format">FranAudio format</param>
		/// <returns>miniaudio format</returns>
		static ma_format ConvertFormat(Sound::WaveFormat format);
	};
}
