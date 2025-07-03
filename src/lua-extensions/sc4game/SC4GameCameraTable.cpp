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

#include "SC4GameCameraTable.h"
#include "cIGZApp.h"
#include "cIGZGraphicSystem2.h"
#include "cIGZWin.h"
#include "cISC43DRender.h"
#include "cISC4App.h"
#include "cISC4City.h"
#include "cS3DVector3.h"
#include "cRZAutoRefCount.h"
#include "GlobalPointers.h"
#include "LuaIGZCommandParameterSetHelper.h"
#include "SafeInt.hpp"

static constexpr uint32_t kCommandID_GetViewTarget = 0x0BB9853C;
static constexpr uint32_t kCommandID_SetViewTarget = 0x8BB756A4;
static constexpr uint32_t kCommandID_RotateCW = 0x6A935CC9;
static constexpr uint32_t kCommandID_RotateCCW = 0x6A935CCC;
static constexpr uint32_t kCommandID_TakeSnapshot = 0x2BB17574;

namespace
{
	bool TakeSnapshot(
		const cIGZString& filePath,
		uint32_t x,
		uint32_t y,
		uint32_t width,
		uint32_t height)
	{
		bool result = false;

		if (spCommandServer)
		{
			cRZAutoRefCount<cIGZCommandParameterSet> commandInput;
			cRZAutoRefCount<cIGZCommandParameterSet> commandOutput;

			if (spCommandServer->CreateCommandParameterSet(commandInput.AsPPObj())
				&& spCommandServer->CreateCommandParameterSet(commandOutput.AsPPObj()))
			{
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 0, filePath);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 1, x);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 2, y);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 3, width);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 4, height);

				spCommandServer->ExecuteCommand(
					kCommandID_TakeSnapshot,
					commandInput,
					commandOutput);

				const cIGZVariant* pStatus = commandOutput->GetParameter(cIGZCommandParameterSet::kStatusParameterID);

				if (pStatus)
				{
					result = pStatus->GetValSint32() == 0;
				}
			}
		}

		return result;
	}
}

SC4GameCameraTable::SC4GameCameraTable()
	: SC4GameTableBase("camera")
{
}

void SC4GameCameraTable::RegisterTableFunctions(
	cISC4GameDataRegistry::cEntry* parentNode,
	cISC4GameDataRegistry& registry)
{
	registry.AddChildEntry(
		&sGetCurrentCell,
		"get_current_cell",
		sGetCurrentCell,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sGetCurrentPosition,
		"get_current_position",
		sGetCurrentPosition,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sJumpToCell,
		"jump_to_cell",
		sJumpToCell,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sJumpToPosition,
		"jump_to_position",
		sJumpToPosition,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sRotateClockwise,
		"rotate_clockwise",
		sRotateClockwise,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sRotateCounterClockwise,
		"rotate_counter_clockwise",
		sRotateCounterClockwise,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);

	registry.AddChildEntry(
		&sTakeSnapshot,
		"take_snapshot",
		sTakeSnapshot,
		parentNode,
		cISC4GameDataRegistry::EntryFlagCallFunction,
		this);
}

bool SC4GameCameraTable::sGetCurrentCell(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	int32_t cellX = 0;
	int32_t cellZ = 0;

	if (spCommandServer)
	{
		cRZAutoRefCount<cIGZCommandParameterSet> commandInput;
		cRZAutoRefCount<cIGZCommandParameterSet> commandOutput;

		if (spCommandServer->CreateCommandParameterSet(commandInput.AsPPObj())
			&& spCommandServer->CreateCommandParameterSet(commandOutput.AsPPObj()))
		{
			LuaIGZCommandParameterSetHelper::SetValue(commandInput, 0, cRZBaseString("cell"));

			spCommandServer->ExecuteCommand(
				kCommandID_GetViewTarget,
				commandInput,
				commandOutput);

			const cIGZVariant* pXVariant = commandOutput->GetParameter(0);
			const cIGZVariant* pZVariant = commandOutput->GetParameter(1);

			if (pXVariant && pZVariant)
			{
				cellX = pXVariant->GetValSint32();
				cellZ = pZVariant->GetValSint32();
			}
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 0, cellX);
	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 1, cellZ);
	return true;
}

bool SC4GameCameraTable::sGetCurrentPosition(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cS3DVector3 position;

	if (spCommandServer)
	{
		cRZAutoRefCount<cIGZCommandParameterSet> commandInput;
		cRZAutoRefCount<cIGZCommandParameterSet> commandOutput;

		if (spCommandServer->CreateCommandParameterSet(commandInput.AsPPObj())
			&& spCommandServer->CreateCommandParameterSet(commandOutput.AsPPObj()))
		{
			LuaIGZCommandParameterSetHelper::SetValue(commandInput, 0, cRZBaseString("position"));

			spCommandServer->ExecuteCommand(
				kCommandID_GetViewTarget,
				commandInput,
				commandOutput);

			const cIGZVariant* pXVariant = commandOutput->GetParameter(0);
			const cIGZVariant* pYVariant = commandOutput->GetParameter(1);
			const cIGZVariant* pZVariant = commandOutput->GetParameter(2);

			if (pXVariant && pYVariant && pZVariant)
			{
				position.fX = pXVariant->GetValFloat32();
				position.fY = pYVariant->GetValFloat32();
				position.fZ = pZVariant->GetValFloat32();
			}
		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 0, position.fX);
	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 1, position.fY);
	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 2, position.fZ);
	return true;
}

bool SC4GameCameraTable::sJumpToCell(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	auto entry = pRegistry->GetEntryID(pKey);
	auto pThis = static_cast<SC4GameCameraTable*>(pRegistry->GetEntryContext(entry));

	uint32_t cellX = 0;
	uint32_t cellZ = 0;

	if (spCity
		&& spCommandServer
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, cellX)
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 1, cellZ))
	{
		if (spCity->CellIsInBounds(cellX, cellZ))
		{
			cRZAutoRefCount<cIGZCommandParameterSet> commandInput;
			cRZAutoRefCount<cIGZCommandParameterSet> commandOutput;

			if (spCommandServer->CreateCommandParameterSet(commandInput.AsPPObj())
				&& spCommandServer->CreateCommandParameterSet(commandOutput.AsPPObj()))
			{
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 0, cRZBaseString("cell"));
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 1, cellX);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 2, cellZ);

				spCommandServer->ExecuteCommand(
					kCommandID_SetViewTarget,
					commandInput,
					commandOutput);
			}
		}
	}

	return true;
}

bool SC4GameCameraTable::sJumpToPosition(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	cS3DVector3 position;

	if (spCity
		&& spCommandServer
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 0, position.fX)
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 1, position.fY)
		&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 2, position.fZ))
	{
		if (spCity->LocationIsInBounds(position.fX, position.fZ))
		{
			cRZAutoRefCount<cIGZCommandParameterSet> commandInput;
			cRZAutoRefCount<cIGZCommandParameterSet> commandOutput;

			if (spCommandServer->CreateCommandParameterSet(commandInput.AsPPObj())
				&& spCommandServer->CreateCommandParameterSet(commandOutput.AsPPObj()))
			{
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 0, cRZBaseString("position"));
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 1, position.fX);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 2, position.fY);
				LuaIGZCommandParameterSetHelper::SetValue(commandInput, 3, position.fZ);

				spCommandServer->ExecuteCommand(
					kCommandID_SetViewTarget,
					commandInput,
					commandOutput);
			}
		}
	}

	return true;
}

bool SC4GameCameraTable::sRotateClockwise(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	if (spCommandServer)
	{
		spCommandServer->ExecuteCommand(kCommandID_RotateCW, nullptr, nullptr);
	}

	return true;
}

bool SC4GameCameraTable::sRotateCounterClockwise(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	if (spCommandServer)
	{
		spCommandServer->ExecuteCommand(kCommandID_RotateCCW, nullptr, nullptr);
	}

	return true;
}

bool SC4GameCameraTable::sTakeSnapshot(
	void* pKey,
	cIGZCommandParameterSet* pInput,
	cIGZCommandParameterSet* pOutput,
	cISC4GameDataRegistry* pRegistry)
{
	bool result = false;

	cRZBaseString filePath;

	if (spView3D
		&& LuaIGZCommandParameterSetHelper::GetString(pInput, 0, filePath)
		&& filePath.Strlen() > 0)
	{
		cISC43DRender* pRender = spView3D->GetRenderer();

		if (pRender)
		{
			uint32_t viewportWidth = 0;
			uint32_t viewportHeight = 0;

			pRender->GetViewportSize(viewportWidth, viewportHeight);

			const uint32_t parameterCount = pInput->GetParameterCount();

			if (parameterCount == 1)
			{
				result = TakeSnapshot(filePath, 0, 0, viewportWidth, viewportHeight);
			}
			else
			{
				uint32_t x = 0;
				uint32_t y = 0;
				uint32_t width = 0;
				uint32_t height = 0;

				if (LuaIGZCommandParameterSetHelper::GetNumber(pInput, 1, x)
					&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 2, y)
					&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 3, width)
					&& LuaIGZCommandParameterSetHelper::GetNumber(pInput, 4, height))
				{
					// Check the requested area fits within the game window.

					if (x < viewportWidth && y < viewportHeight && width > 0 && height > 0)
					{
						uint64_t areaRight = static_cast<uint64_t>(x) + static_cast<uint64_t>(width);
						uint64_t areaBottom = static_cast<uint64_t>(y) + static_cast<uint64_t>(height);

						if (areaRight <= static_cast<uint64_t>(viewportWidth)
							&& areaBottom <= static_cast<uint64_t>(areaBottom))
						{
							result = TakeSnapshot(filePath, x, y, width, height);
						}
					}
				}
			}

		}
	}

	LuaIGZCommandParameterSetHelper::SetValue(pOutput, 1, result);
	return true;
}
