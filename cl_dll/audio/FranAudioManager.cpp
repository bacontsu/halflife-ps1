// FranticDreamer 2022-2026

#include "hud.h"
#include "cl_util.h"
#include "audio/FranAudioManager.hpp"

#include "FranAudioClient/FranAudioClient.hpp"

#include <filesystem>

// Smooth per-frame view transform from V_CalcRefdef (view.cpp).
extern Vector v_origin;
extern Vector v_angles;

void CFranAudioManager::Init()
{
	FranAudioClient::Init(true);
	// Debug Shit
	//FranAudioClient::Wrapper::Backend::LoadAudioFile("FranBase/sound/valve.mp3");
	//FranAudioClient::Wrapper::Backend::PlayAudioFile("FranBase/sound/valve.mp3");

	entityAudioMap.reserve(reservedAudioSlots);
}

void CFranAudioManager::VidInit()
{
	//FranAudioClient::Wrapper::SetBackend(FranAudio::Backend::BackendType::OpenALSoft);
}

void CFranAudioManager::Reset()
{
}

void CFranAudioManager::Shutdown()
{
}

void CFranAudioManager::Draw()
{
	static float lastInvalidationTime = 0.0f;
	static float nextInvalidationTime = 0.0f;

	Vector plrForward = {};
	Vector plrRight = {};
	Vector plrUp = {};

	// Entity Positions
	for (const auto& [entId, soundIds] : entityAudioMap)
	{
		cl_entity_s* entPtr = gEngfuncs.GetEntityByIndex(entId);

		if (entPtr == nullptr)
		{
			// Entity invalid. Skip iteration.
			// Should we just invalidate?
			continue;
		}

		UpdateEntitySounds(gEngfuncs.GetEntityByIndex(entId), soundIds);
	}

	gEngfuncs.pfnAngleVectors(v_angles, plrForward, plrRight, plrUp);

	// Player Transform
	FranAudioClient::Wrapper::Backend::SetListenerTransform(v_origin, plrForward, plrUp);


	if (gEngfuncs.GetClientTime() >= nextInvalidationTime)
	{
		CalculateInvalidEntities();
		lastInvalidationTime = gEngfuncs.GetClientTime();
		nextInvalidationTime = lastInvalidationTime + entityInvalidationInterval;
	}
}

bool CFranAudioManager::CacheSound(const std::string& soundPath)
{
	return FranAudioClient::Wrapper::Backend::LoadAudioFile(soundPath) != SIZE_MAX;
}

size_t CFranAudioManager::EmitSound(const std::string& sample, int entityIndex, float volume, float attenuation, int pitch)
{
	const std::string sampleValveDir = "valve/sound/" + sample;

	if (isLazyLoadingEnabled && !CacheSound(sampleValveDir))
	{
		return SIZE_MAX;
	}

	cl_entity_s* entPtr = gEngfuncs.GetEntityByIndex(entityIndex);

	if (entPtr == nullptr)
	{
		return SIZE_MAX;
	}

	const size_t soundIndex = FranAudioClient::Wrapper::Backend::PlayAudioFile(sampleValveDir);

	if (soundIndex == SIZE_MAX)
	{
		return SIZE_MAX;
	}

	entityAudioMap[entityIndex].emplace_back(soundIndex);
	FranAudioClient::Wrapper::Sound::SetVolume(soundIndex, volume);
	//FranAudioClient::Wrapper::Sound::SetPitch(pitch); // TODO
	FranAudioClient::Wrapper::Sound::SetPosition(soundIndex, entPtr->origin);
	// Without this, OpenAL's default reference distance of 1.0.
	// One GoldSrc unit is an inch, I assume. And it makes gain swing wildly with tiny relative movements.
	FranAudioClient::Wrapper::Sound::SetAttenuation(soundIndex, 0.01f, 100.0f, 500.0f);

	return soundIndex;
}

size_t CFranAudioManager::EmitSound(const std::string& sample, float* origin, float volume, float attenuation, int pitch)
{
	const std::string sampleValveDir = "valve/sound/" + sample;

	if (isLazyLoadingEnabled && !CacheSound(sampleValveDir))
	{
		return SIZE_MAX;
	}

	const size_t soundIndex = FranAudioClient::Wrapper::Backend::PlayAudioFile(sampleValveDir);

	if (soundIndex == SIZE_MAX)
	{
		return SIZE_MAX;
	}

	FranAudioClient::Wrapper::Sound::SetVolume(soundIndex, volume);
	// FranAudioClient::Wrapper::Sound::SetPitch(pitch); // TODO
	FranAudioClient::Wrapper::Sound::SetPosition(soundIndex, origin);
	FranAudioClient::Wrapper::Sound::SetAttenuation(soundIndex, 0.01f, 100.0f, 500.0f);

	return soundIndex;
}

void CFranAudioManager::StopSound(size_t soundIndex)
{
	FranAudioClient::Wrapper::Sound::Stop(soundIndex);
}

void CFranAudioManager::SetSoundVolume(size_t soundIndex, float volume)
{
	FranAudioClient::Wrapper::Sound::SetVolume(soundIndex, volume);
}

void CFranAudioManager::SetSoundAttenuation(size_t soundIndex, float attenuation)
{
	// TODO
}

void CFranAudioManager::SetSoundPitch(size_t soundIndex, int pitch)
{
	// TODO
}

void CFranAudioManager::UpdateEntitySounds(cl_entity_t* entPtr, const std::vector<size_t> soundIds)
{
	for (const auto& soundId : soundIds)
	{
		if (entPtr == nullptr)
		{
			return;
		}

		FranAudioClient::Wrapper::Sound::SetPosition(soundId, entPtr->origin);

		
		//if (!FranAudioClient::Wrapper::Sound::IsValid(soundId))
		//{
		//	entPtr->mouth.mouthopen = 0;
		//}

		/*
		// TODO: Mouth stuff
		if (mouthSoundBuffer == nullptr)
		{
			entPtr->mouth.mouthopen = 0;
			return;
		}

		ALint sampleOffset;
		FranAudio_AlFunction(alGetSourcei, sourceHandle, AL_SAMPLE_OFFSET, &sampleOffset);

		// auto& sound = FindPrecachedSoundSingle(sampleDir);

		entPtr->mouth.mouthopen = mouthSoundBuffer[sampleOffset] / 3;
		*/
	}
}

void CFranAudioManager::CalculateInvalidEntities()
{
	// TODO: Get rid of this. Use events.

	for (auto it = entityAudioMap.begin(); it != entityAudioMap.end();)
	{
		const int entId = it->first;
		auto& sounds = it->second;

		cl_entity_s* entPtr = gEngfuncs.GetEntityByIndex(entId);

		// Entity is invalid. Get rid of it.
		if (!entPtr)
		{
			it = entityAudioMap.erase(it);
			continue;
		}

		// Entity is valid. Remove invalid sounds
		sounds.erase
		(
			std::remove_if(sounds.begin(),sounds.end(),
				[](size_t soundId)
				{
					return !FranAudioClient::Wrapper::Sound::IsValid(soundId);
				}),sounds.end()
		);

		++it;
	}
}
