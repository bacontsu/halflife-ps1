// FranticDreamer 2022-2025
#pragma once

#include "Sound/WaveData/WaveData.hpp"

/// <summary>
/// Namespace for Decoder Abstract and Implementations.
/// </summary>
namespace FranAudio::Decoder
{
	/// <summary>
	/// Settings for decoding audio wave data.
	/// </summary>
	class DecodeSettings
	{
	private:
		/// <summary>
		/// If true, the decoded audio data will be kept in memory after reset.
		/// Until either the backend is shutdown or the persistent data is explicitly cleared.
		/// </summary>
		bool persistent;

		/// <summary>
		/// Forced number of channels for decoding audio files.
		/// 0 means no forced number of channels.
		/// </summary>
		char forcedChannels;

		/// <summary>
		/// Forced sample rate for decoding audio files.
		/// 0 means no forced sample rate.
		/// </summary>
		int forcedSampleRate;

		/// <summary>
		/// Forced format for decoding audio files.
		/// WaveFormat::Unknown means no forced format.
		/// </summary>
		FranAudio::Sound::WaveFormat forcedFormat;

	public:
		constexpr DecodeSettings(bool persistent = false, char forcedChannels = 0, int forcedSampleRate = 0, FranAudio::Sound::WaveFormat forcedFormat = FranAudio::Sound::WaveFormat::Unknown) noexcept
			: persistent(persistent), forcedChannels(forcedChannels), forcedSampleRate(forcedSampleRate), forcedFormat(forcedFormat)
		{
		}

		/// <summary>
		/// If true, the decoded audio data will be kept in memory after reset.
		/// Until either the backend is shutdown or the persistent data is explicitly cleared.
		/// </summary>
		/// <returns>Is persistent?</returns>
		constexpr bool IsPersistent() const noexcept
		{
			return persistent;
		}

		/// <summary>
		/// Set whether the decoded audio data will be kept in memory after reset.
		/// Until either the backend is shutdown or the persistent data is explicitly cleared.
		/// </summary>
		/// <param name="isPersistent">New persistency state.</param>
		constexpr void SetPersistent(bool isPersistent) noexcept
		{
			persistent = isPersistent;
		}

		/// <summary>
		/// Get the forced number of channels for decoding audio files.
		/// 0 means no forced number of channels.
		/// </summary>
		/// <returns>Forced number of channels.</returns>
		constexpr char GetForcedChannels() const noexcept
		{
			return forcedChannels;
		}

		/// <summary>
		/// Set the forced number of channels for decoding audio files.
		/// 0 means no forced number of channels.
		/// </summary>
		/// <param name="channels">New forced number of channels.</param>
		constexpr void SetForcedChannels(char channels) noexcept
		{
			forcedChannels = channels;
		}

		/// <summary>
		/// Get the forced sample rate for decoding audio files. 0 means no forced sample rate.
		/// </summary>
		/// <returns>Forced sample rate for decoding audio files.</returns>
		constexpr int GetForcedSampleRate() const noexcept
		{
			return forcedSampleRate;
		}

		/// <summary>
		/// Set the forced sample rate for decoding audio files. 0 means no forced sample rate.
		/// </summary>
		/// <param name="sampleRate">New forced sample rate for decoding audio files.</param>
		constexpr void SetForcedSampleRate(int sampleRate) noexcept
		{
			forcedSampleRate = sampleRate;
		}

		/// <summary>
		/// Get the forced format for decoding audio files. WaveFormat::Unknown means no forced format.
		/// </summary>
		/// <returns>Forced format for decoding audio files.</returns>
		constexpr FranAudio::Sound::WaveFormat GetForcedFormat() const noexcept
		{
			return forcedFormat;
		}

		/// <summary>
		/// Set the forced format for decoding audio files. WaveFormat::Unknown means no forced format.
		/// </summary>
		constexpr void SetForcedFormat(FranAudio::Sound::WaveFormat format) noexcept
		{
			forcedFormat = format;
		}
	};
}
