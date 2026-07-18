// FranticDreamer 2022-2025
#pragma once

#include <string>
#include <iostream>
#include <streambuf>
#include <memory>

#include "FranAudioShared/Containers/Vector.hpp"

namespace FranAudioShared
{
	/// <summary>
	/// Logger for logging messages and errors.
	/// 
	/// To disable logging, define <i>FRANAUDIO_DISABLE_LOGGING</i>.
	/// </summary>
	namespace Logger
	{
		// ========================
		// Logging Functions
		// ========================

		/// <summary>
		/// Logs a message to the console.
		/// 
		/// If line is true, a new line will be added after the message.
		/// 
		/// <para>
		/// This log message will have a preceding '[INFO]' tag
		/// and will be printed in cyan.
		/// </para>
		/// 
		/// </summary>
		/// <param name="message">Message to log</param>
		/// <param name="newLine">Should add a newline? (Default = true)</param>
		void LogMessage(const std::string& message, bool newLine = true);

		/// <summary>
		/// Logs an error message to the console.
		/// 
		/// If line is true, a new line will be added after the message.
		/// 
		/// <para>
		/// This log message will have a preceding '[ERROR]' tag 
		/// and will be printed in red.
		/// </para>
		/// 
		/// </summary>
		/// <param name="message">Error message to log</param>
		/// <param name="newLine">Should add a newline? (Default = true)</param>
		void LogError(const std::string& message, bool newLine = true);

		/// <summary>
		/// Logs a warning message to the console.
		/// 
		/// If line is true, a new line will be added after the message.
		/// 
		/// <para>
		/// This log message will have a preceding '[WARNING]' tag
		/// and will be printed in yellow.
		/// </para>
		/// 
		/// </summary>
		/// <param name="message">Warning message to log</param>
		/// <param name="newLine">Should add a newline? (Default = true)</param>
		void LogWarning(const std::string& message, bool newLine = true);

		/// <summary>
		/// Logs a success message to the console.
		/// 
		/// If line is true, a new line will be added after the message.
		/// 
		/// <para>
		/// This log message will have a preceding '[SUCCESS]' tag
		/// and will be printed in green.
		/// </para>
		/// 
		/// </summary>
		/// <param name="message">Success message to log</param>
		/// <param name="newLine">Should add a newline? (Default = true)</param>
		void LogSuccess(const std::string& message, bool newLine = true);

		/// <summary>
		/// Logs a generic message to the console.
		/// 
		/// If line is true, a new line will be added after the message.
		/// 
		/// <para>
		/// This log message will have no preceding tag
		/// and will be printed in default colour.
		/// </para>
		/// 
		/// </summary>
		/// <param name="message">Message to log</param>
		/// <param name="newLine">Should add a newline? (Default = true)</param>
		void LogGeneric(const std::string& message, bool newLine = true);

		/// <summary>
		/// Logs a separator to the console.
		/// </summary>
		/// <param name="newLine">Should add a newline? (Default = true)</param>
		void LogSeperator(bool newLine = true);

		/// <summary>
		/// Logs a new line to the console.
		/// </summary>
		void LogNewline();

		// ========================
		// Console
		// ========================

		/// <summary>
		/// Represents an entry in a console log, including the message text and the number of times it has been logged.
		/// </summary>
		struct ConsoleEntry
		{
			std::string text;
			//size_t count = 1; // Number of times this entry has been logged
		};

		/// <summary>
		/// A basic output console manager. Manages console entries.
		/// </summary>
		class FranAudioConsole
		{
		private:
			FranAudioShared::Containers::Vector<ConsoleEntry> entries;

		public:
			FranAudioConsole();

			/// <summary>
			/// Appends a new entry to the list or increments the count of the last entry if it matches the given text.
			/// Appends if a new entry added to the the list, moves entry to end if an existing entry was updated.
			/// </summary>
			/// <param name="entry">The text of the entry to write or update.</param>
			void Write(const std::string& entry);

			/// <summary>
			/// Retrieves a constant reference to the collection of console entries.
			/// </summary>
			/// <returns>A constant reference to a list containing ConsoleEntry objects.</returns>
			const FranAudioShared::Containers::Vector<ConsoleEntry>& GetEntries() const;

			/// <summary>
			/// Clears the entries.
			/// </summary>
			void Clear();

			/// <summary>
			/// Removes the element at the specified index.
			/// </summary>
			/// <param name="index">Index of the element to remove.</param>
			void Remove(size_t index);
		};

		/// <summary>
		/// A custom stream buffer that redirects output to a FranAudioConsole instance.
		/// Buffering output and writing it when a newline is encountered or the buffer exceeds a certain size.
		/// </summary>
		class ConsoleStreamBuffer : public std::streambuf
		{
		private:
			FranAudioConsole& console;
			std::string buffer;

		protected:
			int overflow(int c) override;

		public:
			ConsoleStreamBuffer() = delete;
			ConsoleStreamBuffer(const ConsoleStreamBuffer&) = delete;
			ConsoleStreamBuffer(FranAudioConsole& console);

			FranAudioConsole& GetConsole();

		};

		// =========================
		// Console Routing
		// =========================

		inline std::unique_ptr<std::ostream> customOstream = nullptr;
		inline ConsoleStreamBuffer* customStreamBuffer = nullptr;
		inline std::streambuf* defaultCoutBuffer = nullptr;
		inline std::streambuf* defaultCerrBuffer = nullptr;

		/// <summary>
		/// Redirects the standard output and error streams to the specified console stream buffer.
		/// </summary>
		/// <param name="consoleBuffer">A pointer to the ConsoleStreamBuffer to which console output will be routed.</param>
		void RouteToConsole(ConsoleStreamBuffer* consoleBuffer);

		/// <summary>
		/// Restores the default output stream buffers.
		/// </summary>
		void RestoreDefault();
	};
}