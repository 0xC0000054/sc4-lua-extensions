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

#include "LuaHelper.h"
#include "cRZBaseString.h"

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
				SetResultFromIGZVariant(pLua, pVariant);
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
						SetResultFromIGZVariant(pLua, pParameterSet->GetParameter(id));
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
				SetResultFromIGZVariant(pLua, pParameterSet->GetParameter(id));
			}
		}
	}
	else
	{
		pLua->PushNil();
	}
}

void LuaHelper::SetResultFromIGZVariant(cISCLua* pLua, const cIGZVariant* pVariant)
{
	const cIGZVariant::Type type = static_cast<cIGZVariant::Type>(pVariant->GetType());

	switch (type)
	{
	case cIGZVariant::Type::Bool:
		PushValue(pLua, pVariant->GetValBool());
		break;
	case cIGZVariant::Type::BoolArray:
		CreateLuaArray(pLua, pVariant->RefBool(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Uint8:
		PushValue(pLua, pVariant->GetValUint8());
		break;
	case cIGZVariant::Type::Uint8Array:
		CreateLuaArray(pLua, pVariant->RefUint8(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Sint8:
		PushValue(pLua, pVariant->GetValSint8());
		break;
	case cIGZVariant::Type::Sint8Array:
		CreateLuaArray(pLua, pVariant->RefSint8(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Uint16:
		PushValue(pLua, pVariant->GetValUint16());
		break;
	case cIGZVariant::Type::Uint16Array:
		CreateLuaArray(pLua, pVariant->RefUint16(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Sint16:
		PushValue(pLua, pVariant->GetValSint16());
		break;
	case cIGZVariant::Type::Sint16Array:
		CreateLuaArray(pLua, pVariant->RefSint16(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Uint32:
		PushValue(pLua, pVariant->GetValUint32());
		break;
	case cIGZVariant::Type::Uint32Array:
		CreateLuaArray(pLua, pVariant->RefUint32(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Sint32:
		PushValue(pLua, pVariant->GetValSint32());
		break;
	case cIGZVariant::Type::Sint32Array:
		CreateLuaArray(pLua, pVariant->RefSint32(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Uint64:
		PushValue(pLua, pVariant->GetValUint64());
		break;
	case cIGZVariant::Type::Uint64Array:
		CreateLuaArray(pLua, pVariant->RefUint64(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Sint64:
		PushValue(pLua, pVariant->GetValSint64());
		break;
	case cIGZVariant::Type::Sint64Array:
		CreateLuaArray(pLua, pVariant->RefSint64(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Float32:
		PushValue(pLua, pVariant->GetValFloat32());
		break;
	case cIGZVariant::Type::Float32Array:
		CreateLuaArray(pLua, pVariant->RefFloat32(), pVariant->GetCount());
		break;
	case cIGZVariant::Type::Float64:
		PushValue(pLua, pVariant->GetValFloat64());
		break;
	case cIGZVariant::Type::Float64Array:
		CreateLuaArray(pLua, pVariant->RefFloat64(), pVariant->GetCount());
		break;
	default:
		cRZBaseString asString;
		pVariant->GetValString(asString);

		pLua->PushLString(asString.Data(), asString.Strlen());
		break;
	}
}
