// FranticDreamer 2022-2025
#pragma once

#include <print>
#include <iostream>
#include <string>
#include <string_view>
#include <ranges>
#include <optional>

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#endif

#include "FranAudioShared/Containers/Vector.hpp"

#include "FranAudioShared/Logger/Logger.hpp"

/// <summary>
/// Network related functionality for FranAudio.
/// </summary>
namespace FranAudioShared::Network
{
	// Constants
	// These should match on the server AND client

	constexpr size_t messageBufferSize = 1024;
	constexpr auto listenAddress = "127.0.0.1"; // localhost
	constexpr unsigned short listenPort = 31069; // Hehe
	constexpr const int messageTimeout = 10000; // Reasonable file read time

	/// <summary>
	/// Marker placed right after the '$' of a message to tell the server not to send a reply.
	/// The client does not wait for a reply on these, so setters cost no round trip.
	/// Example: "$!sound-set_position|3|1|2|3"
	/// </summary>
	constexpr char noReplyMarker = '!';

	/// <summary>
	/// How long the server waits for a (re)connection before exiting when started with the "-autoexit" flag.
	/// So when the game process ends, the OS closes its socket and the server times out.
	/// </summary>
	constexpr int serverIdleTimeoutSeconds = 10;

	/// <summary>
	/// Represents a network function with a name and parameters.
	///
	/// This is used for client-server communication in FranAudio.
	/// The function name is the first part of the message, followed by parameters separated by '|'.
	/// Example: "$[functionName]|[param1]|[param2]|...|[paramN]"
	/// </summary>
	struct NetworkFunction
	{
		std::string functionName;
		FranAudioShared::Containers::Vector<std::string> params;

		/// <summary>
		/// Default constructor.
		/// </summary>
		NetworkFunction() = default;

		/// <summary>
		/// Default constructor.
		/// </summary>
		NetworkFunction(std::string functionName, FranAudioShared::Containers::Vector<std::string> params)
			: functionName(std::move(functionName)), params(std::move(params))
		{

		}

		/// <summary>
		/// Parse constructor.
		/// </summary>
		NetworkFunction(const std::string& input)
		{
			auto parsedFunction = ParseFunction(input);
			functionName = std::move(parsedFunction.functionName);
			params = std::move(parsedFunction.params);
		}

		/// <summary>
		/// Parse a string input to create a NetworkFunction object.
		///
		/// - Input format should be like this:
		/// "$[functionName]|[param1]|[param2]|...|[paramN]"
		/// </summary>
		///
		/// <param name="input">Input string, generally the received network message</param>
		/// <returns>Parsed NetworkFunction object</returns>
		static NetworkFunction ParseFunction(const std::string& input)
		{
			if (input.size() < 2 || input.front() != '$')
			{
#if !(defined FRANAUDIO_CLIENT_DISABLE_LOGGING || defined FRANAUDIO_SERVER_DISABLE_LOGGING)
				std::println("NetworkFunction ParseInput: Invalid input format for network function");
#endif
				return {};
			}

			auto body = std::string_view(input).substr(1);

			// Split on '|' and convert each part to std::string_view
			auto partsView = body | std::views::split('|') | std::views::transform([](auto&& subrange)
			{
				return std::string(subrange.begin(), subrange.end()); // safe construction
			});

			std::vector<std::string> parts(std::ranges::begin(partsView), std::ranges::end(partsView));

			// Empty parm fix
			while (!parts.empty() && parts.back().empty())
			{
				parts.pop_back();
			}

			if (parts.empty() || parts[0].empty())
			{
#if !(defined FRANAUDIO_CLIENT_DISABLE_LOGGING || defined FRANAUDIO_SERVER_DISABLE_LOGGING)
				std::println("NetworkFunction ParseInput: Missing function name for network function");
#endif
				return {};
			}

			NetworkFunction result;
			result.functionName = std::move(parts[0]); // first = function name
			result.params.assign(parts.begin() + 1, parts.end());

			return result;
		}

		/// <summary>
		/// Convert the NetworkFunction object to a string representation.
		///
		/// - Output format will be like this:
		/// "$[functionName]|[param1]|[param2]|...|[paramN]"
		/// </summary>
		///
		/// <returns>String representation of the NetworkFunction</returns>
		std::string ToString() const
		{
			std::string result = "$" + functionName;

			for (const auto& param : params)
			{
				result += "|" + param;
			}

			return result;
		}

	};

#ifdef _WIN32
	namespace Win32Helpers
	{
		/// <summary>
		/// Converts a 64-bit unsigned integer from host byte order to big-endian byte order.
		/// </summary>
		/// <param name="data">The 64-bit unsigned integer value in host byte order</param>
		/// <returns>The 64-bit unsigned integer value in big-endian byte order</returns>
		inline uint64_t hostToBE64(const uint64_t data)
		{
			const uint32_t hi = htonl(static_cast<uint32_t>(data >> 32));
			const uint32_t lo = htonl(static_cast<uint32_t>(data & 0xFFFFFFFFu));
			return (static_cast<uint64_t>(lo) << 32) | hi;
		}

		/// <summary>
		/// Converts a 64-bit unsigned integer from big-endian byte order to the host's native byte order.
		/// </summary>
		/// <param name="data">The 64-bit unsigned integer in big-endian byte order</param>
		/// <returns>The 64-bit unsigned integer converted to the host's native byte order</returns>
		inline uint64_t be64ToHost(const uint64_t data)
		{
			const uint32_t lo = ntohl(static_cast<uint32_t>(data >> 32));
			const uint32_t hi = ntohl(static_cast<uint32_t>(data & 0xFFFFFFFFu));
			return (static_cast<uint64_t>(hi) << 32) | lo;
		}

		/// <summary>
		/// Tries to send all data over a socket, ensuring the entire buffer is transmitted.
		/// </summary>
		/// <param name="socket">The socket through which data will be sent</param>
		/// <param name="data">Pointer to the buffer containing the data to send</param>
		/// <param name="length">The number of bytes to send from the buffer</param>
		/// <returns>True if all data was sent successfully, false if an error occurred</returns>
		inline bool SendAll(const SOCKET socket, const char* data, size_t length)
		{
			while (length > 0)
			{
				const int sent = send(socket, data, static_cast<int>(std::min(length, static_cast<size_t>(INT32_MAX))), 0);

				if (sent == SOCKET_ERROR)
				{
					FranAudioShared::Logger::LogError("SendAll: Connection closed or error happened in send.");
					return false;
				}

				data += sent;
				length -= sent;
			}
			return true;
		}

		/// <summary>
		/// Tries to receive exactly the specified number of bytes from a socket.
		/// </summary>
		/// <param name="socket">The socket from which to receive data</param>
		/// <param name="data">A pointer to the buffer where the received data will be stored</param>
		/// <param name="length">The exact number of bytes to receive</param>
		/// <returns>True if the exact number of bytes was received, false if the connection was closed or an error occurred</returns>
		inline bool RecvExactly(const SOCKET socket, char* data, size_t length)
		{
			while (length > 0)
			{
				const int received = recv(socket, data, static_cast<int>(std::min(length, static_cast<size_t>(INT32_MAX))), 0);

				if (received <= 0)
				{
					FranAudioShared::Logger::LogError("RecvExactly: Connection closed or error happened in receive.");
					return false;
				}

				data += received;
				length -= received;
			}
			return true;
		}

		/// <summary>
		/// Tries to send a data frame over a socket, including the data size in big-endian format followed by the data.
		/// </summary>
		/// <param name="socket">The socket through which the frame will be sent</param>
		/// <param name="data">The data to be sent as the frame</param>
		/// <returns>True if the frame was sent successfully, false if an error occurred</returns>
		inline bool SendFrame(const SOCKET socket, const std::string& data)
		{
			const uint64_t length = hostToBE64(static_cast<uint64_t>(data.size()));
			return SendAll(socket, reinterpret_cast<const char*>(&length), sizeof(length)) && SendAll(socket, data.data(), data.size());
		}

		/// <summary>
		/// Tries to receive a data frame from a socket, reading its length prefix and contents.
		/// </summary>
		/// <param name="socket">The socket from which to receive the frame</param>
		/// <returns>An optional string containing the received frame data, or std::nullopt if an error occurred</returns>
		inline std::optional<std::string> RecvFrame(const SOCKET socket)
		{
			uint64_t bigEndianLength = 0;
			if (!RecvExactly(socket, reinterpret_cast<char*>(&bigEndianLength), sizeof(bigEndianLength)))
			{
				return std::nullopt;
			}

			const uint64_t length = be64ToHost(bigEndianLength);

			if (length > (1ull << 32))
			{
				FranAudioShared::Logger::LogError("RecvFrame: Frame too large.");
				return std::nullopt;
			}

			std::string buffer;
			buffer.resize(static_cast<size_t>(length));
			if (!RecvExactly(socket, buffer.data(), buffer.size()))
			{
				return std::nullopt;
			}

			return buffer;
		}
	}
#endif

}

