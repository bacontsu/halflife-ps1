// FranticDreamer 2022-2025
#pragma once

#include <string>

#include "FranAudioAPI.hpp"

#include "FranAudioShared/Containers/Vector.hpp"

#include "WaveFormats.hpp"

namespace FranAudio::Sound
{
	/// <summary>
	/// Contains decoded audio data.
	/// </summary>
	class WaveData
	{
	private:
		/// <summary>
		/// Filename of the audio file.
		/// </summary>
		std::string filename;

		/// <summary>
		/// Unique ID of the wave data. Never reused, like sound IDs.
		/// </summary>
		size_t waveDataID;

		// =========
		// Wave Info
		// =========

		/// <summary>
		/// Audio format.
		/// </summary>
		WaveFormat format;

		/// <summary>
		/// Length of the audio in seconds.
		/// </summary>
		double length;

		/// <summary>
		/// Number of channels.
		/// </summary>
		char channels;

		/// <summary>
		/// Sample rate.
		/// </summary>
		int sampleRate;

		// =========
		// Frames
		// =========

		/// <summary>
		/// Audio samples.
		/// </summary>
		SampleFrameContainer frames;

		/// <summary>
		/// Bits per frame * channels.
		/// </summary>
		unsigned char frameSize;

		// =========
		// Flags
		// =========

		/// <summary>
		/// If true, wave data will be kept in memory when the cache is cleared.
		/// Not const so WaveData stays assignable; there is intentionally no setter.
		/// </summary>
		bool isPersistent;

	public:
		/// <summary>
		/// Default constructor.
		/// </summary>
		FRANAUDIO_API WaveData(const std::string& filename = "", size_t waveDataID = 0, WaveFormat format = WaveFormat::Unknown, double length = 0.0, int channels = 0, int sampleRate = 0, bool isPersistent = false);

		// =========
		// Setters
		// =========

		/// <summary>
		/// Sets the filename of the audio file including path if any.
		/// </summary>
		/// <param name="filename">Input File</param>
		FRANAUDIO_API void SetFilename(const std::string& filename);

		/// <summary>
		/// Sets the unique ID of the wave data.
		/// </summary>
		/// <param name="id">Input ID</param>
		FRANAUDIO_API void SetWaveDataID(size_t id);

		/// <summary>
		/// Sets the audio format of the wave data.
		/// </summary>
		/// <param name="format">Format to set to</param>
		FRANAUDIO_API void SetFormat(WaveFormat format);

		/// <summary>
		/// Sets the length of the audio in seconds. 
		/// </summary>
		/// <param name="length">Length of the audio in seconds</param>
		FRANAUDIO_API void SetLength(double length);

		/// <summary>
		/// Sets the number of audio channels.
		/// </summary>
		/// <example>1 = Mono, 2 = Stereo, etc.</example>
		/// <param name="channels">Number of channels to set</param>
		FRANAUDIO_API void SetChannels(char channels);

		/// <summary>
		/// Sets the sample rate.
		/// </summary>
		/// <param name="sampleRate">Desired sample rate in samples per second (Hz)</param>
		FRANAUDIO_API void SetSampleRate(int sampleRate);

		// =========
		// Getters
		// =========

		/// <summary>
		/// Get the filename of the audio file including path if any.
		/// </summary>
		/// <returns>Filename of the audio file</returns>
		[[nodiscard]] FRANAUDIO_API const std::string& GetFilename() const;

		/// <summary>
		/// Get the unique ID of the wave data.
		/// </summary>
		/// <returns>Wave data ID</returns>
		[[nodiscard]] FRANAUDIO_API size_t GetWaveDataID() const;

		/// <summary>
		/// Get the audio format of the wave data.
		/// </summary>
		/// <returns>Wave format</returns>
		[[nodiscard]] FRANAUDIO_API WaveFormat GetFormat() const;

		/// <summary>
		/// Get the length of the audio in seconds.
		/// </summary>
		/// <returns>Length of the audio in seconds</returns>
		[[nodiscard]] FRANAUDIO_API double GetLength() const;

		/// <summary>
		/// Get the number of audio channels.
		/// </summary>
		/// <example>1 = Mono, 2 = Stereo, etc.</example>
		/// <returns>Number of channels</returns>
		[[nodiscard]] FRANAUDIO_API char GetChannels() const;

		/// <summary>
		/// Get the sample rate.
		/// </summary>
		/// <returns>Sample rate in samples per second (Hz)</returns>
		[[nodiscard]] FRANAUDIO_API int GetSampleRate() const;

		/// <summary>
		/// Get if wave data is persistent.
		/// If true, wave data will be kept in memory when the cache is cleared.
		/// </summary>
		/// <returns>If wave data is persistent</returns>
		[[nodiscard]] FRANAUDIO_API bool IsPersistent() const;

		// =========
		// Frame Stuff
		// =========

		/// <summary>
		/// Size in frames.
		/// Aka: Total Frames / Channels.
		/// </summary>
		[[nodiscard]] const FRANAUDIO_API size_t SizeInFrames() const;

		/// <summary>
		/// Set size per frame.
		/// Aka: Bits per frame * channels.
		/// </summary>
		FRANAUDIO_API void SetFrameSize(unsigned char frameSize);

		/// <summary>
		/// Size per frame.
		/// Aka: Bits per frame * channels.
		/// </summary>
		[[nodiscard]] FRANAUDIO_API unsigned char GetFrameSize() const;

		/// <summary>
		/// Set the frames manually.
		/// This will set frame vector.
		/// </summary>
		/// <param name="frames"></param>
		FRANAUDIO_API void SetFrames(const SampleFrameContainer& frames);

		/// <summary>
		/// Get a reference to frames vector.
		/// </summary>
		FRANAUDIO_API SampleFrameContainer& GetFramesRef();

		/// <summary>
		/// Get frames.
		/// </summary>
		[[nodiscard]] const FRANAUDIO_API SampleFrameContainer& GetFrames() const;

		// =========
		// Utilities
		// =========

		/// <summary>
		/// Mixes multi-channel audio to mono.
		/// </summary>
		/// <remarks>If the audio is already mono, this function does nothing.</remarks>
		FRANAUDIO_API void MixToMono();

		/// <summary>
		/// Mixes mono audio to stereo.
		/// </summary>
		/// <remarks>
		/// <para>
		/// If the audio is already stereo, this function does nothing.
		/// </para>
		/// <para>
		/// If the audio has more than 2 channels, this function will first mix it to mono, then to stereo.
		/// </para>
		/// </remarks>
		FRANAUDIO_API void MixToStereo();
	};
}