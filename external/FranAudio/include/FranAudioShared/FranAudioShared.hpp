// FranticDreamer 2022-2025
#pragma once

#include <memory>
#include <array>
#include <tuple>
#include <span>
#include <string>

#include "FranAudioAPI.hpp"

/// <summary>
/// Contains shared code for the FranAudio library itself, Server and Client.
/// </summary>
/// <remarks>
/// This is a separate project to avoid circular dependencies.
/// Contains classes and functions for Networking, Logging, Containers and other shared utilities.
/// </remarks>
namespace FranAudioShared
{
	// ========================
	// General Data Start
	// ========================

	/// <summary>
	/// 3D Vector type used for sound positions, listener positions, etc.
	/// </summary>
	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	/// <summary>
	/// Tuple-like type for listener transform, containing position, forward vector, and up vector.
	/// </summary>
	struct ListenerTransform
	{
		Vector3 position;
		Vector3 forward;
		Vector3 up;
	};

	/// <summary>
	/// Tuple-like type for listener orientation, containing forward vector and up vector.
	/// </summary>
	struct ListenerOrientation
	{
		Vector3 forward;
		Vector3 up;
	};

	/// <summary>
	/// Tuple-like type for sound attenuation parameters, containing rolloff factor, minimum distance, and maximum distance.
	/// </summary>
	struct SoundAttenuation
	{
		float rolloffFactor;
		float minDistance;
		float maxDistance;
	};

	/// <summary>
	/// Default attenuation parameters applied to newly played 3D sounds.
	/// minDistance is the radius in which the sound plays at full volume.
	/// Beyond it, gain falls off as minDistance / distance (inverse model, rolloffFactor = 1).
	/// </summary>
	inline constexpr SoundAttenuation defaultSoundAttenuation = { 1.0f, 1.0f, 10000.0f };

	/// <summary>
	/// The group every sound belongs to unless explicitly assigned to another one.
	/// There is no "ungrouped" sounds. Group operations on this name will modify all unassigned sounds.
	/// </summary>
	inline constexpr auto defaultSoundGroupName = "__ungrpd__";

	// ========================
	// General Data End
	// ========================
}
