// FranticDreamer 2022-2025
#pragma once

#include <memory>
#include <span>

#include "FranAudioAPI.hpp"

#include "FranAudioShared/FranAudioShared.hpp"

#include "WaveData/WaveData.hpp"

/// <summary>
/// Namespace for Sound related classes and functions.
/// </summary>
namespace FranAudio::Sound
{
	/// <summary>
	/// Represents a sound instance.
	/// 
	/// Basically a struct, combined with a wrapper around
	/// sound instances and backend interaction. 
	/// </summary>
	class Sound
	{
	private:
		/// <summary>
		/// ID of the sound.
		/// </summary>
		size_t soundID;

		/// <summary>
		/// Unique wave data ID of the sound.
		///
		/// This is used to identify the wave data associated with the sound.
		/// SIZE_MAX for sounds without cached wave data (e.g. streamed sounds).
		/// </summary>
		size_t waveDataID;

	public:
		/// <summary>
		/// Constructor.
		/// </summary>
		FRANAUDIO_API Sound(size_t soundID = SIZE_MAX, size_t waveDataID = SIZE_MAX);

		/// <summary>
		/// Destructor.
		/// </summary>
		virtual FRANAUDIO_API ~Sound() = default;

		/// <summary>
		/// Check if the sound is valid.
		/// </summary>
		FRANAUDIO_API bool IsValid() const;
	
		/// <summary>
		/// Get the sound ID.
		/// </summary>
		FRANAUDIO_API size_t GetSoundID() const;

		/// <summary>
		/// Get the unique wave data ID.
		/// </summary>
		FRANAUDIO_API size_t GetWaveDataID() const;

		/// <summary>
		/// Stop and clear the sound.
		/// </summary>
		FRANAUDIO_API void Stop() const;

		/// <summary>
		/// Set whether the sound is paused or not.
		/// </summary>
		/// <param name="isPaused">True to pause the sound, false to resume playback</param>
		FRANAUDIO_API void SetPaused(bool isPaused) const;

		/// <summary>
		/// Check if the sound is paused.
		/// </summary>
		FRANAUDIO_API bool IsPaused() const;

		/// <summary>
		/// Set the volume of the sound.
	 	/// </summary>
 		/// <param name="volume">Volume to set the sound to (0.0 - 1.0)</param>
		FRANAUDIO_API void SetVolume(float volume) const;

		/// <summary>
		/// Get the volume of the sound.
 		/// </summary>
 		/// <returns>Current volume of the sound (0.0 - 1.0)</returns>
		FRANAUDIO_API float GetVolume() const;

		/// <summary>
		/// Set the pitch of the sound.
		/// </summary>
		/// <param name="pitch">Pitch to set the sound to (1.0 = normal pitch)</param>
		FRANAUDIO_API void SetPitch(float pitch) const;

		/// <summary>
		/// Get the pitch of the sound.
		/// </summary>
		FRANAUDIO_API float GetPitch() const;

		/// <summary>
		/// Set whether the sound loops.
		/// </summary>
		/// <param name="looping">True to loop the sound, false to play it once</param>
		FRANAUDIO_API void SetLooping(bool looping) const;

		/// <summary>
		/// Check if the sound loops.
		/// </summary>
		FRANAUDIO_API bool IsLooping() const;

		/// <summary>
		/// Assign the sound to a group (e.g. "sfx", "music", "voice").
		/// </summary>
		/// <param name="groupName">Name of the group</param>
		FRANAUDIO_API void SetGroup(const std::string& groupName) const;

		/// <summary>
		/// Get the group the sound belongs to.
		/// </summary>
		/// <returns>Group name, or an empty string if the sound is not in a group</returns>
		FRANAUDIO_API std::string GetGroup() const;

		/// <summary>
		/// Set the position of the sound.
 		/// </summary>
		/// <param name="position">New position of the sound</param>
		FRANAUDIO_API void SetPosition(const FranAudioShared::Vector3& position) const;

		/// <summary>
		/// Get the position of the sound.
		/// </summary>
		/// <returns>Position of the sound</returns>
		FRANAUDIO_API FranAudioShared::Vector3 GetPosition() const;

		/// <summary>
		/// Set the attenuation parameters of the sound.
		/// </summary>
		/// <param name="rolloffFactor">Rolloff factor to set the sound attenuation to</param>
		/// <param name="minDistance">Minimum distance to set the sound attenuation to</param>
		/// <param name="maxDistance">Maximum distance to set the sound attenuation to</param>
		FRANAUDIO_API void SetAttenuation(float rolloffFactor, float minDistance, float maxDistance) const;

		/// <summary>
		/// Get the attenuation parameters of the sound.
		/// </summary>
		/// <returns>Attenuation parameters of the sound</returns>
		FRANAUDIO_API FranAudioShared::SoundAttenuation GetAttenuation() const;

	};
}
