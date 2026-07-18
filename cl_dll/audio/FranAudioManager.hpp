// FranticDreamer 2022-2026

#ifndef FRANAUDIO_MANAGER_H
#define FRANAUDIO_MANAGER_H

#include <string>
#include <unordered_map>

#include "const.h"
#include "cl_entity.h"

/*
/// <summary>
/// Struct to hold audio information associated with an entity.
/// </summary>
struct FranAudioInfo
{
	size_t audioInstanceID;
	int channel;

	FranAudioInfo(size_t id, int channel)
		: audioInstanceID(id), channel(channel)
	{
	}
};
*/

/// <summary>
/// Audio Manager Class for FranAudio integration.
/// Handles entity-audio interactions and updates.
/// 
/// An instance of this class is created in the HUD module.
/// 
/// Should we use CHudBase?
/// </summary>
class CFranAudioManager
{
private:
	static constexpr bool isLazyLoadingEnabled = true;
	static constexpr int reservedAudioSlots = 128;
	static constexpr float entityInvalidationInterval = 10.0f; // Seconds per invalidation check

	/// <summary>
	/// Map of entity IDs to audio instance IDs.
	/// Used to track which audio is associated with which entity positions.
	/// </summary>
	std::unordered_map<int, std::vector<size_t>> entityAudioMap;

public:
	//CFranAudioManager();
	//~CFranAudioManager();

	/// <summary>
	/// Initialise the audio manager.
	/// </summary>
	void Init();

	/// <summary>
	/// Video initialisation for the audio manager.
	/// Usually called when a level is loaded.
	/// </summary>
	void VidInit();

	/// <summary>
	/// Reset the audio manager.
	/// </summary>
	void Reset();

	/// <summary>
	/// Shutdown the audio manager.
	/// </summary>
	void Shutdown();

	/// <summary>
	/// Update the audio manager. Called every frame.
	/// NOT Thread-Safe.
	/// </summary>
	void Draw();

	/// <summary>
	/// Load a sound file into memory for future use.
	/// </summary>
	/// <param name="soundPath">Path to the sound file.</param>
	/// <returns>True if the sound was successfully cached, false otherwise.</returns>
	bool CacheSound(const std::string& soundPath);

	/// <summary>
	/// Emit a sound from a specific entity.
	/// </summary>
	/// <param name="sample">The sound sample to play.</param>
	/// <param name="entityIndex">The entity index emitting the sound.</param>
	/// <param name="volume">Volume of the sound (0.0 to 1.0).</param>
	/// <param name="attenuation">Attenuation factor for the sound.</param>
	/// <param name="pitch">Pitch adjustment for the sound.</param>
	/// <returns>The unique sound index if successful, SIZE_MAX otherwise.</returns>
	size_t EmitSound(const std::string& sample, int entityIndex = -1, float volume = 1.0f, float attenuation = ATTN_NORM, int pitch = PITCH_NORM);

	/// <summary>
	/// Emit a sound from a location.
	/// </summary>
	/// <param name="sample">The sound sample to play.</param>
	/// <param name="origin">The origin point from which the sound is emitted.</param>
	/// <param name="volume">Volume of the sound (0.0 to 1.0).</param>
	/// <param name="attenuation">Attenuation factor for the sound.</param>
	/// <param name="pitch">Pitch adjustment for the sound.</param>
	/// <returns>The unique sound index if successful, SIZE_MAX otherwise.</returns>
	size_t EmitSound(const std::string& sample, float* origin, float volume = 1.0f, float attenuation = ATTN_NORM, int pitch = PITCH_NORM);


	/// <summary>
	/// Stop a sound given its unique index.
	/// </summary>
	/// <param name="soundIndex">The unique sound index.</param>
	void StopSound(size_t soundIndex);

	/// <summary>
	/// Set the volume of a sound given its unique index.
	/// </summary>
	/// <param name="soundIndex">The unique sound index.</param>
	/// <param name="volume">The new volume (0.0 to 1.0).</param>
	void SetSoundVolume(size_t soundIndex, float volume);

	/// <summary>
	/// Set the attenuation of a sound given its unique index.
	/// </summary>
	/// <param name="soundIndex">The unique sound index.</param>
	/// <param name="attenuation">The new attenuation factor.</param>
	void SetSoundAttenuation(size_t soundIndex, float attenuation);

	/// <summary>
	/// Set the pitch of a sound given its unique index.
	/// </summary>
	/// <param name="soundIndex">The unique sound index.</param>
	/// <param name="pitch">The new pitch value.</param>
	void SetSoundPitch(size_t soundIndex, int pitch);

private:
	void UpdateEntitySounds(cl_entity_t* entPtr, const std::vector<size_t> soundIds);

	void CalculateInvalidEntities();
};

#endif // FRANAUDIO_MANAGER_H