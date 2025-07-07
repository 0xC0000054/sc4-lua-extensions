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

#include "SC4GameCityTable.h"
#include "cIGZAllocatorService.h"
#include "cIGZWin.h"
#include "cISC4ViewInputControl.h"
#include "cISTETerrain.h"
#include "cRZAutoRefCount.h"
#include "cRZBaseString.h"
#include "cS3DVector3.h"
#include "GlobalPointers.h"
#include "GZServPtrs.h"
#include "LuaIGZCommandParameterSetHelper.h"
#include "SC4Rect.h"
#include "SC4VersionDetection.h"

namespace
{
	class cSC4ViewInputControlPlaceLot : public cISC4ViewInputControl
	{
		uint8_t unknown[0x154];
	};
	static_assert(sizeof(cSC4ViewInputControlPlaceLot) == 0x158);

	typedef cSC4ViewInputControlPlaceLot* (__thiscall* pfn_cSC4ViewInputControlPlaceLot_ctor)(
		cSC4ViewInputControlPlaceLot* pThis,
		int32_t isBuildingPlop,
		uint32_t lotExemplarID,
		uint32_t buildingExemplarID,
		bool useMonopoly);
	typedef bool(__thiscall* pfn_cSC4ViewInputControlPlaceLot_PlaceLot)(
		cSC4ViewInputControlPlaceLot* pThis,
		const cS3DVector3& position,
		int32_t facing,
		bool useLotPlopEffects,
		SC4Rect<int32_t>& outLotBounds);

	static const pfn_cSC4ViewInputControlPlaceLot_ctor cSC4ViewInputControlPlaceLot_ctor = reinterpret_cast<pfn_cSC4ViewInputControlPlaceLot_ctor>(0x4c0430);
	static const pfn_cSC4ViewInputControlPlaceLot_PlaceLot cSC4ViewInputControlPlaceLot_PlaceLot = reinterpret_cast<pfn_cSC4ViewInputControlPlaceLot_PlaceLot>(0x4c1740);

	cRZAutoRefCount<cSC4ViewInputControlPlaceLot> CreateViewInputControlPlaceLot(uint32_t lotExemplarID, uint32_t buildingExemplarID)
	{
		cRZAutoRefCount<cSC4ViewInputControlPlaceLot> instance;

		cIGZAllocatorServicePtr pAllocatorService;

		if (pAllocatorService)
		{
			auto pControl = static_cast<cSC4ViewInputControlPlaceLot*>(pAllocatorService->Allocate(sizeof(cSC4ViewInputControlPlaceLot)));

			if (pControl)
			{
				cSC4ViewInputControlPlaceLot_ctor(pControl, 0, lotExemplarID, buildingExemplarID, false);

				instance = pControl;
			}
		}

		return instance;
	}

	bool IsInRange(int32_t value, int32_t min, int32_t max)
	{
		return value >= min && value <= max;
	}
}


SC4GameCityTable::SC4GameCityTable()
	: SC4GameTableBase("city")
{
}

void SC4GameCityTable::RegisterTableFunctions(
	cISC4GameDataRegistry::cEntry* parentNode,
	cISC4GameDataRegistry& registry)
{
	registry.AddChildEntry(
		&sGetCityName,
		"get_city_name",
		sGetCityName,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);
	registry.AddChildEntry(
		&sSetCityName,
		"set_city_name",
		sSetCityName,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetMayorName,
		"get_mayor_name",
		sGetMayorName,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);
	registry.AddChildEntry(
		&sSetMayorName,
		"set_mayor_name",
		sSetMayorName,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sPlaceLot,
		"place_lot",
		sPlaceLot,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);
}

bool SC4GameCityTable::sGetCityName(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString name;

	if (spCity)
	{
		spCity->GetCityName(name);
	}

	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 1, name);
	return true;
}

bool SC4GameCityTable::sSetCityName(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString newName;

	if (spCity && LuaIGZCommandParameterSetHelper::GetString(pInput, 0, newName))
	{
		if (newName.Strlen() > 0)
		{
			spCity->SetCityName(newName);
		}
	}

	return true;
}

bool SC4GameCityTable::sGetMayorName(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString name;

	if (spCity)
	{
		spCity->GetMayorName(name);
	}

	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 1, name);
	return true;
}

bool SC4GameCityTable::sSetMayorName(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cRZBaseString newName;

	if (spCity && LuaIGZCommandParameterSetHelper::GetString(pInput, 0, newName))
	{
		if (newName.Strlen() > 0)
		{
			spCity->SetMayorName(newName);
		}
	}

	return true;
}

bool SC4GameCityTable::sPlaceLot(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	bool result = false;

	if (SC4VersionDetection::IsVersion641() && spCity)
	{
		int32_t cellX = 0;
		int32_t cellZ = 0;
		int32_t facing = 0;
		uint32_t lotExemplarID = 0;

		if (LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, cellX)
			&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 1, cellZ)
			&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 2, facing)
			&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 3, lotExemplarID))
		{
			if (spCity->CellIsInBounds(cellX, cellZ)
				&& IsInRange(facing, 0 , 3)
				&& lotExemplarID != 0)
			{
				uint32_t buildingExemplarID = 0;

				// The buildingExemplarID parameter is optional, but callers can use it if they want
				// a specific variant for a lot that is used by multiple building families.
				// When the value is 0, lots with multiple building families will use a randomly
				// picked building.
				if (!LuaIGZCommandParameterSetHelper::GetNumber(pInput, 4, buildingExemplarID))
				{
					buildingExemplarID = 0;
				}

				cISTETerrain* pTerrain = spCity->GetTerrain();

				if (pTerrain)
				{
					cS3DVector3 position{};

					spCity->CellCenterToPosition(cellX, cellZ, position.fX, position.fZ);

					position.fY = pTerrain->GetAltitude(position.fX, position.fZ);

					auto placeLotControl = CreateViewInputControlPlaceLot(lotExemplarID, buildingExemplarID);

					if (placeLotControl)
					{
						placeLotControl->Init();

						SC4Rect<int32_t> outLotBounds{};

						result = cSC4ViewInputControlPlaceLot_PlaceLot(
							placeLotControl,
							position,
							facing,
							false,
							outLotBounds);

						placeLotControl->Shutdown();
					}
				}
			}
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 1, result);
	return true;
}
