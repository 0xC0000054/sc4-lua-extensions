/*
 * This file is part of sc4-lua-extensions, a DLL Plugin for SimCity 4
 * that extends the game's Lua system with new functions.
 *
 * Copyright (C) 2025 Nicholas Hayes
 *
 * sc4-lua-extensions is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * sc4-lua-extensions is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with sc4-lua-extensions.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "Logger.h"
#include <Windows.h>

namespace
{
#ifdef _DEBUG
	void PrintToDebugOutput(const char* lpOutputString)
	{
		OutputDebugStringA(lpOutputString);
	}

	void PrintLineToDebugOutput(const char* line)
	{
		PrintToDebugOutput(line);
		PrintToDebugOutput("\n");
	}
#endif // _DEBUG
}

Logger& Logger::GetInstance()
{
	static Logger logger;

	return logger;
}

void Logger::Flush()
{
	if (initialized && logFile)
	{
		logFile.flush();
	}
}

Logger::Logger() : initialized(false), logFile(), logLevel(LogLevel::Error)
{
}

Logger::~Logger()
{
	initialized = false;
}

void Logger::Init(std::filesystem::path logFilePath, LogLevel options)
{
	if (!initialized)
	{
		initialized = true;

		// Open the log file in binary mode to allow UTF-8 text to be written without modification.
		// UTF-8 is the native encoding of SC4.
		logFile.open(logFilePath, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
		logLevel = options;
	}
}

bool Logger::IsEnabled(LogLevel level) const
{
	return logLevel >= level;
}

void Logger::Write(LogLevel level, const char* const message)
{
	if (!IsEnabled(level))
	{
		return;
	}

	WriteCore(message);
}

void Logger::WriteLogFileHeader(const char* const text)
{
	if (initialized && logFile)
	{
		logFile << text << std::endl;
	}
}

void Logger::WriteLine(LogLevel level, const char* const message)
{
	if (!IsEnabled(level))
	{
		return;
	}

	WriteLineCore(message);
}

void Logger::WriteLineFormatted(LogLevel level, const char* const format, ...)
{
	if (!IsEnabled(level))
	{
		return;
	}

	va_list args;
	va_start(args, format);

	va_list argsCopy;
	va_copy(argsCopy, args);

	int formattedStringLength = std::vsnprintf(nullptr, 0, format, argsCopy);

	va_end(argsCopy);

	if (formattedStringLength > 0)
	{
		size_t formattedStringLengthWithNull = static_cast<size_t>(formattedStringLength) + 1;

		std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(formattedStringLengthWithNull);

		std::vsnprintf(buffer.get(), formattedStringLengthWithNull, format, args);

		WriteLineCore(buffer.get());
	}

	va_end(args);
}

void Logger::WriteCore(const char* const message)
{
	if (initialized && logFile)
	{
#ifdef _DEBUG
		PrintToDebugOutput(message);
#endif // _DEBUG

		logFile << message;
	}
}

void Logger::WriteLineCore(const char* const message)
{
	if (initialized && logFile)
	{
#ifdef _DEBUG
		PrintLineToDebugOutput(message);
#endif // _DEBUG

		logFile << message << '\n';
	}
}