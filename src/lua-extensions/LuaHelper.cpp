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

#include "LuaHelper.h"
#include "LuaIGZVariantHelper.h"

void LuaHelper::SetResultFromIGZCommandParameterSet(cISCLua* pLua, cIGZCommandParameterSet* pParameterSet)
{
	if (pParameterSet)
	{
		uint32_t parameterCount = pParameterSet->GetParameterCount();

		if (parameterCount == 0)
		{
			pLua->PushNil();
		}
		else if (parameterCount == 1)
		{
			const uint32_t id = pParameterSet->GetFirstParameterID();
			const cIGZVariant* pVariant = pParameterSet->GetParameter(id);

			if (id == cIGZCommandParameterSet::kStatusParameterID)
			{
				pLua->PushBoolean(pVariant->GetValSint32() == 0);
			}
			else
			{
				LuaIGZVariantHelper::PushValuesToLua(pLua, pVariant);
			}
		}
		else
		{
			pParameterSet->RemoveParameter(cIGZCommandParameterSet::kStatusParameterID);

			parameterCount = pParameterSet->GetParameterCount();

			if (parameterCount > 1)
			{
				if (parameterCount < static_cast<uint32_t>(std::numeric_limits<int32_t>::max() - 1))
				{
					// Lua uses a one-based index for arrays.
					int32_t luaTableIndex = 1;

					for (uint32_t id = pParameterSet->GetFirstParameterID(); id != UINT_MAX; id = pParameterSet->GetNextParameterID(id))
					{
						LuaIGZVariantHelper::PushValuesToLua(pLua, pParameterSet->GetParameter(id));
						pLua->RawSetI(-2, luaTableIndex++);
					}
				}
				else
				{
					pLua->PushNil();
				}
			}
			else
			{
				const uint32_t id = pParameterSet->GetFirstParameterID();
				LuaIGZVariantHelper::PushValuesToLua(pLua, pParameterSet->GetParameter(id));
			}
		}
	}
	else
	{
		pLua->PushNil();
	}
}
