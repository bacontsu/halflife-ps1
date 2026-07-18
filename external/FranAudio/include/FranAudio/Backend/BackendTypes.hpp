// FranticDreamer 2022-2025
#pragma once

#include <string_view>

namespace FranAudio::Backend
{
	/// <summary>
	/// Possible backend types.
	/// </summary>
	enum class BackendType
	{
		None = 0,
		miniaudio,
		OpenALSoft,
	};

	/// <summary>
	/// An array of string literals representing the names of available backend types.
	/// </summary>
	inline const char* BackendTypeNames[] =
	{
		"None",
		"MiniAudio",
		"OpenALSoft",
	};
}
