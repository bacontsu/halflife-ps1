// FranticDreamer 2022-2025
#pragma once

#include "miniaudio.h"

#include "Decoder/Decoder.hpp"
#include "Backend/BackendTypes.hpp"

namespace FranAudio::Decoder
{
	/// <summary>
	/// miniaudio Decoder.
	/// </summary>
	class miniaudio : public Decoder
	{
	private:
		/// <summary>
		/// Is this decoder standalone?
		/// 
		/// This is used to determine if the decoder is independent of the backend.
		/// If the backend is not miniaudio, this decoder will handle main miniaudio engine.
		/// </summary>
		bool isStandalone = false;

		ma_engine engine = {};
		ma_engine_config engineConfig = {};
		ma_device device = {};
		ma_device_config deviceConfig = {};
		ma_decoder_config defaultDecoderConfig = {};

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
			return DecoderType::miniaudio;
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
		virtual bool FRANAUDIO_API DecodeAudioFile(const std::string& filename, FranAudio::Sound::WaveData& targetWaveData, FranAudio::Backend::Backend& caller, const DecodeSettings& settings = {}) override;

		/// <summary>
		/// Gets the supported audio formats in lower case.
		/// </summary>
		/// <returns>List of supported audio formats in lower case.</returns>
		virtual const FRANAUDIO_API std::span<const std::string_view> GetSupportedAudioFormats() const override;
	};
}