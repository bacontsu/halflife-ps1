// FranticDreamer 2022-2025
#pragma once

#include <string_view>

namespace FranAudio::Decoder
{
	/// <summary>
	/// Possible decoder types.
	/// </summary>
	enum class DecoderType
	{
		None = 0,
		miniaudio,
		libnyquist,
	};

	/// <summary>
	/// An array of string literals representing the names of available decoder types.
	/// </summary>
	inline const char* DecoderTypeNames[] =
	{
		"None",
		"miniaudio",
		"libnyquist",
	};
}
