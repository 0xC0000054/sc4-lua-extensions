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

#include "SC4GameBudgetTableExtensions.h"
#include "cIGZCommandParameterSet.h"
#include "cISC4DepartmentBudget.h"
#include "cISC4GameDataRegistry.h"
#include "cRZBaseVariant.h"
#include "GlobalPointers.h"
#include "Logger.h"

namespace
{
	bool get_department_total_expense(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry)
	{
		int64_t total = 0;

		if (pInput->GetParameterCount() == 1)
		{
			cIGZVariant* pVariant = pInput->GetParameter(0);

			if (pVariant)
			{
				uint32_t id = pVariant->GetValUint32();

				const cISC4DepartmentBudget* pDepartment = spBudgetSim->GetDepartmentBudget(id);

				if (pDepartment)
				{
					total = pDepartment->GetTotalExpenses();
				}
			}
		}

		cRZBaseVariant outVariant(total);

		pOutput->SetParameter(1, outVariant);
		return true;
	}

	bool get_department_total_income(
		void* pKey,
		cIGZCommandParameterSet* pInput,
		cIGZCommandParameterSet* pOutput,
		cISC4GameDataRegistry* pRegistry)
	{
		int64_t total = 0;

		if (pInput->GetParameterCount() == 1)
		{
			cIGZVariant* pVariant = pInput->GetParameter(0);

			if (pVariant)
			{
				uint32_t id = pVariant->GetValUint32();

				const cISC4DepartmentBudget* pDepartment = spBudgetSim->GetDepartmentBudget(id);

				if (pDepartment)
				{
					total = pDepartment->GetTotalIncome();
				}
			}
		}

		cRZBaseVariant outVariant(total);

		pOutput->SetParameter(1, outVariant);
		return true;
	}
}

void SC4GameBudgetTableExtensions::Register(cISC4GameDataRegistry& registry)
{
	Logger& logger = Logger::GetInstance();

	// Maxis already defined the sc4game.budget category.
	// They used the class instance cast to cIGZUnknown as the entry identifier
	// for the category nodes.

	cRZAutoRefCount<cIGZUnknown> budgetNodeKey;

	spBudgetSim->QueryInterface(GZIID_cIGZUnknown, budgetNodeKey.AsPPVoid());

	auto parentNode = registry.GetEntryID(budgetNodeKey);

	if (parentNode)
	{
		registry.AddChildEntry(
			&get_department_total_expense,
			"get_department_total_expense",
			get_department_total_expense,
			parentNode,
			cISC4GameDataRegistry::EntryFlagCallFunction,
			nullptr);

		logger.WriteLine(LogLevel::Info, "Registered the sc4game.budget.get_department_total_expense function.");

		registry.AddChildEntry(
			&get_department_total_income,
			"get_department_total_income",
			get_department_total_income,
			parentNode,
			cISC4GameDataRegistry::EntryFlagCallFunction,
			nullptr);

		logger.WriteLine(LogLevel::Info, "Registered the sc4game.budget.get_department_total_income function.");
	}
}
