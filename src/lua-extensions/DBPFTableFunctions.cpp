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

#include "DBPFTableFunctions.h"
#include "cGZPersistResourceKey.h"
#include "cIGZPersistDBSegment.h"
#include "cIGZPersistDBSegmentMultiPackedFiles.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "cISCResExemplar.h"
#include "cISCResExemplarCohort.h"
#include "cRZBaseString.h"
#include "GlobalPointers.h"
#include "LuaFunctionRegistration.h"
#include "LuaHelper.h"
#include "SCLuaUtil.h"
#include "StringResourceKey.h"
#include "StringResourceManager.h"
#include <array>

namespace
{
	int32_t get_cohort_property_value(lua_State* pState)
	{
		constexpr uint32_t kCohortTypeID = 0x05342861;

		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		uint32_t groupID = 0;
		uint32_t instanceID = 0;
		uint32_t propertyID = 0;

		if (LuaHelper::FunctionHasParameterCount(lua, 3)
			&& spResourceManager
			&& LuaHelper::GetNumber(lua, 1, groupID)
			&& LuaHelper::GetNumber(lua, 2, instanceID)
			&& LuaHelper::GetNumber(lua, 3, propertyID))
		{
			const cGZPersistResourceKey key(kCohortTypeID, groupID, instanceID);

			cRZAutoRefCount<cISCResExemplarCohort> cohort;

			if (spResourceManager->GetResource(key, GZIID_cISCResExemplarCohort, cohort.AsPPVoid(), 0, nullptr))
			{
				const cISCProperty* pProperty = cohort->AsISCPropertyHolder()->GetProperty(propertyID);

				if (pProperty)
				{
					LuaHelper::SetResultFromIGZVariant(lua, pProperty->GetPropertyValue());
				}
				else
				{
					lua->PushNil();
				}
			}
			else
			{
				lua->PushNil();
			}
		}
		else
		{
			lua->PushNil();
		}

		return 1;
	}

	int32_t get_exemplar_property_value(lua_State* pState)
	{
		constexpr uint32_t kExemplarTypeID = 0x6534284A;

		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		uint32_t groupID = 0;
		uint32_t instanceID = 0;
		uint32_t propertyID = 0;

		if (LuaHelper::FunctionHasParameterCount(lua, 3)
			&& spResourceManager
			&& LuaHelper::GetNumber(lua, 1, groupID)
			&& LuaHelper::GetNumber(lua, 2, instanceID)
			&& LuaHelper::GetNumber(lua, 3, propertyID))
		{
			const cGZPersistResourceKey key(kExemplarTypeID, groupID, instanceID);

			cRZAutoRefCount<cISCResExemplar> exemplar;

			if (spResourceManager->GetResource(key, GZIID_cISCResExemplar, exemplar.AsPPVoid(), 0, nullptr))
			{
				const cISCProperty* pProperty = exemplar->AsISCPropertyHolder()->GetProperty(propertyID);

				if (pProperty)
				{
					LuaHelper::SetResultFromIGZVariant(lua, pProperty->GetPropertyValue());
				}
				else
				{
					lua->PushNil();
				}
			}
			else
			{
				lua->PushNil();
			}
		}
		else
		{
			lua->PushNil();
		}

		return 1;
	}

	int32_t resource_exists(lua_State* pState)
	{
		bool result = false;

		cRZAutoRefCount<cISCLua> lua = SCLuaUtil::GetISCLuaFromFunctionState(pState);

		cGZPersistResourceKey key;

		if (LuaHelper::FunctionHasParameterCount(lua, 3)
			&& spResourceManager
			&& LuaHelper::GetNumber(lua, 1, key.type)
			&& LuaHelper::GetNumber(lua, 2, key.group)
			&& LuaHelper::GetNumber(lua, 3, key.instance))
		{
			result = spResourceManager->TestForKey(key);
		}

		lua->PushBoolean(result);
		return 1;
	}
}

void DBPFTableFunctions::Register(cISCLua* pLua)
{
	constexpr std::array<LuaFunctionRegistration::LuaFunctionInfo, 3> functions =
	{
		LuaFunctionRegistration::LuaFunctionInfo("get_cohort_property_value", get_cohort_property_value),
		LuaFunctionRegistration::LuaFunctionInfo("get_exemplar_property_value", get_exemplar_property_value),
		LuaFunctionRegistration::LuaFunctionInfo("resource_exists", resource_exists),
	};

	LuaFunctionRegistration::RegisterFunctions(pLua, "dbpf", functions.data(), functions.size());
}