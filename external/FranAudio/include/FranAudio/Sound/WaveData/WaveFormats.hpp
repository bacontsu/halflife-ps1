// FranticDreamer 2022-2025
#pragma once

#include <cstdint>
#include <variant>
#include <algorithm>

#include "FranAudioShared/Containers/Vector.hpp"

namespace FranAudio::Sound
{
	using U8Sample = uint8_t;
	using S16Sample = int16_t;
	using FloatSample = float;

	using U8SampleContainer = FranAudioShared::Containers::Vector<U8Sample>;
	using S16SampleContainer = FranAudioShared::Containers::Vector<S16Sample>;
	using FloatSampleContainer = FranAudioShared::Containers::Vector<FloatSample>;

	using SampleFrameContainer = std::variant<U8SampleContainer, S16SampleContainer, FloatSampleContainer>;

	/// <summary>
	/// Possible audio wave formats.
	/// </summary>
	enum class WaveFormat : uint8_t
	{
		Unknown = 0,	///<summary> Unknown format, used for errors, wildcards etc. </summary>
		PCM_8bit,		///<summary> Unsigned 8-bit PCM  </summary>
		PCM_16bit,		///<summary> Signed 16-bit PCM </summary>
		PCM_24bit,		///<summary> Signed 24-bit PCM </summary>
		PCM_32bit,		///<summary> Signed 32-bit PCM </summary>
		IEEE_FLOAT,		///<summary> 32-bit IEEE floating point </summary>
		IEEE_DOUBLE		///<summary> 64-bit IEEE floating point. Not really supported, use with caution. </summary>
	};

	/// <summary>
	/// An array of string literals representing the names of available wave formats.
	/// </summary>
	inline const char* WaveFormatNames[] =
	{
		"Unknown",
		"PCM_8bit",
		"PCM_16bit",
		"PCM_24bit",
		"PCM_32bit",
		"IEEE_FLOAT",
		"IEEE_DOUBLE"
	};

	/// <summary>
	/// Type mapping for wave formats.
	/// </summary>
	template<WaveFormat Format>
	struct WaveFormatType_s;

	template<> struct WaveFormatType_s<WaveFormat::Unknown> { using type = void; };
	template<> struct WaveFormatType_s<WaveFormat::PCM_8bit> { using type = U8Sample; };
	template<> struct WaveFormatType_s<WaveFormat::PCM_16bit> { using type = S16Sample; };
	template<> struct WaveFormatType_s<WaveFormat::PCM_24bit> { using type = FloatSample; };	// Expanded to float.
	template<> struct WaveFormatType_s<WaveFormat::PCM_32bit> { using type = FloatSample; };
	template<> struct WaveFormatType_s<WaveFormat::IEEE_FLOAT> { using type = FloatSample; };
	template<> struct WaveFormatType_s<WaveFormat::IEEE_DOUBLE> { using type = FloatSample; };	// Downcasted to float.

	/// <summary>
	/// Type mapping for wave formats.
	/// </summary>
	template<WaveFormat Format>
	using WaveFormatType = typename WaveFormatType_s<Format>::type;

	/// <summary>
	/// Get the size of a wave format type.
	/// </summary>
	/// <param name="format">Wave format to get the size of</param>
	/// <returns>Size of the wave format type in bytes</returns>
	constexpr size_t GetWaveFormatSize(WaveFormat format)
	{
		switch (format)
		{
		case WaveFormat::PCM_8bit:    return sizeof(WaveFormatType<WaveFormat::PCM_8bit>);
		case WaveFormat::PCM_16bit:   return sizeof(WaveFormatType<WaveFormat::PCM_16bit>);
		case WaveFormat::PCM_24bit:   return sizeof(WaveFormatType<WaveFormat::PCM_24bit>);
		case WaveFormat::PCM_32bit:   return sizeof(WaveFormatType<WaveFormat::PCM_32bit>);
		case WaveFormat::IEEE_FLOAT:  return sizeof(WaveFormatType<WaveFormat::IEEE_FLOAT>);
		case WaveFormat::IEEE_DOUBLE: return sizeof(WaveFormatType<WaveFormat::IEEE_DOUBLE>);
		default:                      return 0;
		}
	}

	constexpr SampleFrameContainer CreateWaveFormatContainer(WaveFormat format)
	{
		switch (format)
		{
		case WaveFormat::PCM_8bit:    return U8SampleContainer{};
		case WaveFormat::PCM_16bit:   return S16SampleContainer{};
		case WaveFormat::PCM_24bit:   return FloatSampleContainer{};
		case WaveFormat::PCM_32bit:   return FloatSampleContainer{};
		case WaveFormat::IEEE_FLOAT:  return FloatSampleContainer{};
		case WaveFormat::IEEE_DOUBLE: return FloatSampleContainer{};
		default:                      return SampleFrameContainer{};
		}
	}

	template<typename T>
	inline constexpr T ConvertSample(FloatSample f);

	template<> inline constexpr U8Sample ConvertSample<uint8_t>(FloatSample f)
	{
		return static_cast<U8Sample>(std::clamp(f * 127.5f + 128.0f, 0.0f, 255.0f));
	}

	template<> inline constexpr S16Sample ConvertSample<int16_t>(FloatSample f)
	{
		return static_cast<S16Sample>(std::clamp(f * 32767.0f, -32768.0f, 32767.0f));
	}

	template<> inline constexpr FloatSample ConvertSample<FloatSample>(FloatSample f)
	{
		return f;
	}

	// ==========================
	// Sample Rate
	// ==========================

	inline constexpr int StandardSampleRates[] =
	{
		8000,
		11025,
		16000,
		22050,
		32000,
		44100,
		48000,
		88200,
		96000,
		176400,
		192000,
		352800,
		384000,
	};
}
