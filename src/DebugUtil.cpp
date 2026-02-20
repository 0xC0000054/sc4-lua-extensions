/*
 * This file is part of sc4-lua-extensions, a DLL Plugin for SimCity 4
 * that extends the game's Lua system with new functions.
 *
 * Copyright (C) 2025, 2026 Nicholas Hayes
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

#include "DebugUtil.h"
#include "cIGZString.h"
#include "cIGZVariant.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "cISC4Occupant.h"
#include "cRZAutoRefCount.h"
#include "StringResourceKey.h"
#include "StringResourceManager.h"
#include <memory>
#include <string>
#include <Windows.h>
#include "wil/result.h"

namespace
{
	std::wstring Utf8ToUtf16(const char* const utf8Str, int utf8Length)
	{
		int utf16Length = MultiByteToWideChar(CP_UTF8, 0, utf8Str, utf8Length, nullptr, 0);

		THROW_LAST_ERROR_IF(utf16Length == 0);

		std::wstring result(static_cast<size_t>(utf16Length), L' ');

		int charsWritten = MultiByteToWideChar(CP_UTF8, 0, utf8Str, utf8Length, result.data(), utf16Length);

		THROW_LAST_ERROR_IF(charsWritten == 0);

		return result;
	}
}

void DebugUtil::PrintLineToDebugOutput(const char* const line)
{
#ifdef _DEBUG
	OutputDebugStringA(line);
	OutputDebugStringA("\n");
#endif // _DEBUG
}

void DebugUtil::PrintLineToDebugOutput(const wchar_t* const line)
{
#ifdef _DEBUG
	OutputDebugStringW(line);
	OutputDebugStringA("\n");
#endif // _DEBUG
}

void DebugUtil::PrintLineToDebugOutputFormatted(const char* const format, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);

	va_list argsCopy;
	va_copy(argsCopy, args);

	int formattedStringLength = std::vsnprintf(nullptr, 0, format, argsCopy);

	va_end(argsCopy);

	if (formattedStringLength > 0)
	{
		size_t formattedStringLengthWithNull = static_cast<size_t>(formattedStringLength) + 1;

		constexpr size_t stackBufferSize = 1024;

		if (formattedStringLengthWithNull >= stackBufferSize)
		{
			std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(formattedStringLengthWithNull);

			std::vsnprintf(buffer.get(), formattedStringLengthWithNull, format, args);

			PrintLineToDebugOutput(buffer.get());
		}
		else
		{
			char buffer[stackBufferSize]{};

			std::vsnprintf(buffer, stackBufferSize, format, args);

			PrintLineToDebugOutput(buffer);
		}
	}

	va_end(args);
#endif // _DEBUG
}

void DebugUtil::PrintLineToDebugOutputFormattedUTF8(const char* const format, ...)
{
	va_list args;
	va_start(args, format);

	va_list argsCopy;
	va_copy(argsCopy, args);

	int formattedStringLength = std::vsnprintf(nullptr, 0, format, argsCopy);

	va_end(argsCopy);

	if (formattedStringLength > 0)
	{
		size_t formattedStringLengthWithNull = static_cast<size_t>(formattedStringLength) + 1;

		std::wstring utf16String;

		constexpr size_t stackBufferSize = 1024;

		if (formattedStringLengthWithNull >= stackBufferSize)
		{
			std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(formattedStringLengthWithNull);

			int charsWritten = std::vsnprintf(buffer.get(), formattedStringLengthWithNull, format, args);

			if (charsWritten > 0)
			{
				utf16String = Utf8ToUtf16(buffer.get(), charsWritten);
			}
		}
		else
		{
			char buffer[stackBufferSize]{};

			int charsWritten =  std::vsnprintf(buffer, stackBufferSize, format, args);

			if (charsWritten > 0)
			{
				utf16String = Utf8ToUtf16(buffer, charsWritten);
			}
		}

		if (!utf16String.empty())
		{
			PrintLineToDebugOutput(utf16String.c_str());
		}
	}

	va_end(args);
}
