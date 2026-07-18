// FranticDreamer 2022-2025
#pragma once

#include "Decoder/Decoder.hpp"

#include "libnyquist/Common.h"
#include "libnyquist/Decoders.h"

namespace FranAudio::Decoder
{
	/// <summary>
	/// Libnyquist Decoder
	/// </summary>
	class libnyquist : public Decoder
	{
	private:
		nqr::NyquistIO soundLoader;

	public:
		//Decoder();
		//~Decoder();

		/// <summary>
		/// Initialise the Decoder.
		/// This is used to initialise the Decoder and set it up for use.
		/// </summary>
		virtual FRANAUDIO_API bool Init() override;

		/// <summary>
		/// Reset the decoder. 
		/// This is used to reset the decoder to its initial state.
		/// </summary>
		virtual FRANAUDIO_API void Reset() override;

		/// <summary>
		/// Shutdown the decoder.
		/// This is used to shut down the decoder and clean up any resources.
		/// </summary>
		virtual FRANAUDIO_API void Shutdown() override;

		/// <summary>
		/// Get the decoder type.
		/// </summary>
		/// <returns>Type of this decoder instance</returns>
		virtual constexpr FRANAUDIO_API DecoderType GetDecoderType() const noexcept override
		{
			return DecoderType::libnyquist;
		}

		/// <summary>
		/// Decode an audio file.
		/// This is used to decode an audio file and store the result in the target WaveData.
		/// </summary>
		/// <remarks>
		/// Important: Audio file MUST exist.
		/// </remarks>
		/// <returns>
		/// True if the decoding was successful, false otherwise.
		/// </returns>
		/// <param name="filename">Path to an existing audio file</param>
		/// <param name="targetWaveData">Target WaveData to store the decoded audio</param>
		/// <param name="caller">Caller backend</param>
		virtual FRANAUDIO_API bool DecodeAudioFile(const std::string& filename, FranAudio::Sound::WaveData& targetWaveData, FranAudio::Backend::Backend& caller, const DecodeSettings& settings = {}) override;

		/// <summary>
		/// Gets the supported audio formats in lower case.
		/// </summary>
		/// <returns>List of supported audio formats in lower case.</returns>
		virtual const FRANAUDIO_API std::span<const std::string_view> GetSupportedAudioFormats() const override;

		// ========================
		// Libnyquist Specific
		// ========================

		/// <summary>
		/// Converts the WaveData to libnyquist AudioData.
		/// </summary>
		/// <param name="waveData">WaveData to convert into libnyquist AudioData</param>
		/// <param name="targetAudioData">Target libnyquist AudioData to store the converted data</param>
		static void ConvertFranDataToNyqData(const FranAudio::Sound::WaveData& waveData, nqr::AudioData& targetAudioData);
	
		/// <summary>
		/// Converts the libnyquist AudioData to WaveData.
		/// </summary>
		/// <param name="audioData">libnyquist AudioData to convert into WaveData</param>
		/// <param name="targetWaveData">Target WaveData to store the converted data</param>
		static void ConvertNyqDataToFranData(const nqr::AudioData& audioData, FranAudio::Sound::WaveData& targetWaveData);

		static FranAudio::Sound::WaveFormat ConvertWaveFormat(const nqr::PCMFormat& format);

		static nqr::PCMFormat ConvertWaveFormat(const FranAudio::Sound::WaveFormat& format);
	};
}
