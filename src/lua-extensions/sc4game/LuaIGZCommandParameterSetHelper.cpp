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

#include "LuaIGZCommandParameterSetHelper.h"
#include "cIGZVariant.h"
#include "cRZBaseVariant.h"

bool LuaIGZCommandParameterSetHelper::GetString(
	cIGZCommandParameterSet* pParameterSet,
	uint32_t parameterIndex,
	cRZBaseString& outValue)
{
	bool result = false;

	const cIGZVariant* pVariant = pParameterSet->GetParameter(parameterIndex);

	if (pVariant)
	{
		result = pVariant->GetValString(outValue);
	}

	return result;
}

void LuaIGZCommandParameterSetHelper::SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, bool value)
{
	cRZBaseVariant variant(value);
	pParameterSet->SetParameter(parameterIndex, variant);
}

void LuaIGZCommandParameterSetHelper::SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, int32_t value)
{
	cRZBaseVariant variant(value);
	pParameterSet->SetParameter(parameterIndex, variant);
}

void LuaIGZCommandParameterSetHelper::SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, uint32_t value)
{
	cRZBaseVariant variant(value);
	pParameterSet->SetParameter(parameterIndex, variant);
}

void LuaIGZCommandParameterSetHelper::SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, float value)
{
	cRZBaseVariant variant(value);
	pParameterSet->SetParameter(parameterIndex, variant);
}

void LuaIGZCommandParameterSetHelper::SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, double value)
{
	cRZBaseVariant variant(value);
	pParameterSet->SetParameter(parameterIndex, variant);
}

void LuaIGZCommandParameterSetHelper::SetValue(cIGZCommandParameterSet* pParameterSet, uint32_t parameterIndex, const cIGZString& value)
{
	cRZBaseVariant variant(value);
	pParameterSet->SetParameter(parameterIndex, variant);
}
