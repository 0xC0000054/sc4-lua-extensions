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

#pragma once
#include "cIGZCommandParameterSet.h"
#include "cIGZVariant.h"
#include "cRZBaseString.h"
#include "SafeInt.hpp"

namespace LuaIGZCommandParameterSetHelper
{
	template <typename T>
	bool GetNumber(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, T& outValue)
	{
		bool result = false;

		const cIGZVariant* pVariant = pParameterSet->GetParameter(parameterIndex);

		if (pVariant)
		{
			double number = 0.0;

			// The native number format of SimCity 4's Lua 5.0 implementation is Float64/double.
			// We perform our own casting for integer types in order to get an error for values
			// that are out of range for the destination type.

			if (pVariant->GetValFloat64(number))
			{
				if constexpr (std::is_same_v<T, double>)
				{
					outValue = number;
					result = true;
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					outValue = static_cast<float>(number);
					result = true;
				}
				else
				{
					result = SafeCast(number, outValue);
				}
			}
		}

		return result;
	}

	bool GetString(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, cRZBaseString& outValue);

	void SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, bool value);
	void SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, int32_t value);
	void SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, uint32_t value);
	void SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, float value);
	void SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, double value);
	void SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, const cIGZString& value);
}