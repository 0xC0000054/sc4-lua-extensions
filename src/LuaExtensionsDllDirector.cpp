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

#include "version.h"
#include "FileSystem.h"
#include "Logger.h"
#include "cIGZApp.h"
#include "cIGZCOM.h"
#include "cIGZFrameWork.h"
#include "cIGZMessageServer2.h"
#include "cIGZMessageTarget.h"
#include "cIGZMessageTarget2.h"
#include "cIGZString.h"
#include "cIGZWin.h"
#include "cISC4AdvisorSystem.h"
#include "cISC4App.h"
#include "cISC4City.h"
#include "cISCLua.h"
#include "cRZMessage2COMDirector.h"
#include "cRZMessage2Standard.h"
#include "cRZBaseString.h"
#include "GlobalPointers.h"
#include "GZServPtrs.h"
#include "LuaPrintFunction.h"
#include "PackageScriptLoadingPatch.h"

#include "GameTableExtensions.h"
#include "SC4GameBudgetTableExtensions.h"
#include "SC4GameCameraTable.h"
#include "SC4GameCityTable.h"
#include "SC4GameLanguageTable.h"
#include "LuaExtensionFunctionTests.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>


static constexpr uint32_t kSC4MessagePostCityInit = 0x26D31EC1;
static constexpr uint32_t kSC4MessagePreCityShutdown = 0x26D31EC2;
static constexpr uint32_t kSC4MessagePostRegionInit = 0xCBB5BB45;
static constexpr uint32_t kSC4MessagePreRegionShutdown = 0x8BB5BB46;
static constexpr uint32_t kMessageCheatIssued = 0x230E27AC;

// Only used to run the Lua testing code in debug builds.
static constexpr uint32_t kSC4MessagePostCityInitComplete = 0xEA8AE29A;

static constexpr std::array<uint32_t, 2> RequiredNotifications =
{
	kSC4MessagePostCityInit,
	kSC4MessagePreCityShutdown,
};

static constexpr uint32_t kLuaExtensionsDirectorID = 0x68AC07EE;

static constexpr uint32_t kShowMaxisLuaDebugTextCheatID = 0x519E8A25;
static constexpr std::string_view kShowMaxisLuaDebugTextCheatStr = "ShowMaxisLuaDebugText";

static constexpr uint32_t kHideMaxisLuaDebugTextCheatID = 0x519E8A26;
static constexpr std::string_view kHideMaxisLuaDebugTextCheatStr = "HideMaxisLuaDebugText";

cIGZCheatCodeManager* spCCM = nullptr;
cRZAutoRefCount<cIGZCommandServer> spCommandServer;
cISC4BudgetSimulator* spBudgetSim;
cISC4City* spCity;
cISC4Simulator* spSimulator;
cRZAutoRefCount<cISC4View3DWin> spView3D;

namespace
{
	void LuaErrorCallback(cIGZLua5Thread* pThread, int32_t errorCode, const char* message)
	{
		Logger& logger = Logger::GetInstance();

		// This error log format is based on the one Maxis used in their built-in Lua script error handler.
		// Maxis removed the logging code from retail builds of the game, so the built-in Lua script error
		// handler just builds an error string that is not used.
		//
		// Unfortunately, Lua scripts added by plugins will not include the script name in the error
		// message, SC4 loads all plugins scripts with the script name as an empty string.
		// But the Lua error message may include the name of the function.

		logger.WriteLine(LogLevel::Error, "Lua script error !\n");

		switch (errorCode)
		{
		case 1:
			logger.WriteLine(LogLevel::Error, "Run-time");
			break;
		case 2:
			logger.WriteLine(LogLevel::Error, "File-related");
			break;
		case 3:
			logger.WriteLine(LogLevel::Error, "Syntax");
			break;
		case 4:
			logger.WriteLine(LogLevel::Error, "Allocation");
			break;
		case 5:
		default:
			logger.WriteLine(LogLevel::Error, "Unknown");
			break;
		}

		if (pThread)
		{
			logger.WriteLineFormatted(LogLevel::Error, "Thread ID: %u", pThread->GetID());
		}
		else
		{
			logger.WriteLine(LogLevel::Error, "Thread ID: Unknown");
		}

		logger.WriteLineFormatted(LogLevel::Error, "Error Type: %s", message);
		logger.WriteLine(LogLevel::Error, "\n==========================================================================================\n");
	}

	void SetLuaErrorCallback(cISC4App* pSC4App, cIGZLua5::ErrorReporterCallback callback)
	{
		// The city advisor/automata scripting system uses the
		// cISCLua instance owned by cISC4App.
		// We take advantage of that and add error logging to help
		// mod authors that write Lua scripts.

		cISCLua* pLuaInterpreter = pSC4App->GetLuaInterpreter();

		if (pLuaInterpreter)
		{
			cIGZLua5* pGZLua5 = pLuaInterpreter->AsIGZLua5();

			if (pGZLua5)
			{
				pGZLua5->SetErrorReporter(callback, true);
			}
		}
	}

	cRZAutoRefCount<cISC4View3DWin> GetView3DWin()
	{
		constexpr uint32_t kGZWin_WinSC4App = 0x6104489a;
		constexpr uint32_t kGZWin_SC4View3DWin = 0x9a47b417;
		constexpr uint32_t kGZIID_cISC4View3DWin = 0xFA47B3F9;

		cRZAutoRefCount<cISC4View3DWin> view3D;

		cISC4AppPtr sc4App;

		cIGZWin* mainWindow = sc4App->GetMainWindow();

		if (mainWindow)
		{
			cIGZWin* pParentWin = mainWindow->GetChildWindowFromID(kGZWin_WinSC4App);

			if (pParentWin)
			{
				if (!pParentWin->GetChildAs(
					kGZWin_SC4View3DWin,
					kGZIID_cISC4View3DWin,
					view3D.AsPPVoid()))
				{
					view3D.Reset();
				}
			}
		}

		return view3D;
	}
}

class LuaExtensionsDllDirector : public cRZMessage2COMDirector
{
public:
	LuaExtensionsDllDirector() : replacedMaxisPrintFunctionPointers(false)
	{
		std::filesystem::path logFilePath = FileSystem::GetPluginLogFilePath();

		Logger& logger = Logger::GetInstance();
		logger.Init(logFilePath, LogLevel::Error);
		logger.WriteLogFileHeader("SC4LuaExtensions v" PLUGIN_VERSION_STR);
	}

	uint32_t GetDirectorID() const
	{
		return kLuaExtensionsDirectorID;
	}

	void PostCityInit(cIGZMessage2Standard* pStandardMsg)
	{
		spCity = static_cast<cISC4City*>(pStandardMsg->GetVoid1());

		if (spCity)
		{
			spBudgetSim = spCity->GetBudgetSimulator();
			spSimulator = spCity->GetSimulator();
			spView3D = GetView3DWin();

			cISCLua* pLua = spCity->GetAdvisorSystem()->GetScriptingContext();

			if (pLua)
			{
				if (!replacedMaxisPrintFunctionPointers)
				{
					LuaPrintFunction::RegisterFallbackPrintFunction(pLua);
				}

				GameTableExtensions::Register(pLua);
				sc4gameBudgetTable.Register(*pLua);
			}

			// The GameDataRegistry class that is used to register the sc4game methods appears
			// to have been intended as a way for C++ classes to add lua functions, it allows
			// a pointer to the class to be stored as user data for the function.

			cISC4GameDataRegistryPtr registry;

			if (registry)
			{
				sc4gameCameraTable.Register(*registry);
				sc4gameCityTable.Register(*registry);
				sc4gameLanguageTable.Register(*registry);
			}
		}
	}

	void PostCityInitComplete()
	{
		// The testing code has to be run in PostCityInitComplete because cISC4View3DWin
		// crashes with a null pointer error when rotating the camera in PostCityInit.

		if (spCity)
		{
			LuaExtensionFunctionTests::Run(spCity->GetAdvisorSystem()->GetScriptingContext());
		}
	}

	void PreCityShutdown()
	{
		spBudgetSim = nullptr;
		spSimulator = nullptr;
		spCity = nullptr;
		spView3D.Reset();
	}

	void PostRegionInit()
	{
		if (spCCM)
		{
			spCCM->AddNotification2(this, 0);
			spCCM->RegisterCheatCode(
				kShowMaxisLuaDebugTextCheatID,
				cRZBaseString(kShowMaxisLuaDebugTextCheatStr.data(), kShowMaxisLuaDebugTextCheatStr.size()));
			spCCM->RegisterCheatCode(
				kHideMaxisLuaDebugTextCheatID,
				cRZBaseString(kHideMaxisLuaDebugTextCheatStr.data(), kHideMaxisLuaDebugTextCheatStr.size()));
		}
	}

	void PreRegionShutdown()
	{
		if (spCCM)
		{
			spCCM->RemoveNotification2(this, 0);
			spCCM->UnregisterCheatCode(kShowMaxisLuaDebugTextCheatID);
			spCCM->UnregisterCheatCode(kHideMaxisLuaDebugTextCheatID);
		}
	}

	void ProcessCheat(cIGZMessage2Standard* pStandardMsg)
	{
		const uint32_t cheatID = static_cast<uint32_t>(pStandardMsg->GetData1());

		switch (cheatID)
		{
		case kShowMaxisLuaDebugTextCheatID:
			LuaPrintFunction::SetPrintMaxisScriptDebugOutput(true);
			break;
		case kHideMaxisLuaDebugTextCheatID:
			LuaPrintFunction::SetPrintMaxisScriptDebugOutput(false);
			break;
		}
	}

	bool DoMessage(cIGZMessage2* pMessage)
	{
		switch (pMessage->GetType())
		{
		case kSC4MessagePostCityInit:
			PostCityInit(static_cast<cIGZMessage2Standard*>(pMessage));
			break;
#ifdef _DEBUG
		case kSC4MessagePostCityInitComplete:
			PostCityInitComplete();
			break;
#endif // _DEBUG
		case kSC4MessagePreCityShutdown:
			PreCityShutdown();
			break;
		case kSC4MessagePostRegionInit:
			PostRegionInit();
			break;
		case kSC4MessagePreRegionShutdown:
			PreRegionShutdown();
			break;
		case kMessageCheatIssued:
			ProcessCheat(static_cast<cIGZMessage2Standard*>(pMessage));
			break;
		}

		return true;
	}

	bool PostAppInit()
	{
		cIGZMessageServer2Ptr pMS2;

		if (pMS2)
		{
			for (const uint32_t& messageID : RequiredNotifications)
			{
				pMS2->AddNotification(this, messageID);
			}

			if (replacedMaxisPrintFunctionPointers)
			{
				// Subscribe to notifications for the region-view cheat codes
				// that control weather the Maxis debug text is included in
				// the log's Lua print output.
				pMS2->AddNotification(this, kSC4MessagePostRegionInit);
				pMS2->AddNotification(this, kSC4MessagePreRegionShutdown);
			}

#ifdef _DEBUG
			pMS2->AddNotification(this, kSC4MessagePostCityInitComplete);
#endif // _DEBUG

		}

		cIGZApp* const pApp = mpFrameWork->Application();

		if (pApp)
		{
			cRZAutoRefCount<cISC4App> sc4App;

			if (pApp->QueryInterface(GZIID_cISC4App, sc4App.AsPPVoid()))
			{
				spCCM = sc4App->GetCheatCodeManager();
				SetLuaErrorCallback(sc4App, LuaErrorCallback);
			}
		}

		cIGZCommandServerPtr commandServer;
		spCommandServer = commandServer;

		sc4gameLanguageTable.PostAppInit(mpFrameWork);

		return true;
	}

	bool PreAppShutdown()
	{
		if (mpFrameWork)
		{
			cIGZApp* const pApp = mpFrameWork->Application();

			if (pApp)
			{
				cRZAutoRefCount<cISC4App> sc4App;

				if (pApp->QueryInterface(GZIID_cISC4App, sc4App.AsPPVoid()))
				{
					SetLuaErrorCallback(sc4App, nullptr);
				}
			}
		}

		spCCM = nullptr;
		spCommandServer.Reset();
		sc4gameLanguageTable.PreAppShutdown();

		return true;
	}

	bool OnStart(cIGZCOM * pCOM)
	{
		replacedMaxisPrintFunctionPointers = LuaPrintFunction::PatchMaxisPrintFunctionPointers();
		PackageScriptLoadingPatch::Install();

		const cIGZFrameWork::FrameworkState state = mpFrameWork->GetState();

		if (state < cIGZFrameWork::kStatePreAppInit)
		{
			mpFrameWork->AddHook(this);
		}
		else
		{
			PreAppInit();
		}
		return true;
	}

	SC4GameBudgetTableExtensions sc4gameBudgetTable;
	SC4GameCameraTable sc4gameCameraTable;
	SC4GameCityTable sc4gameCityTable;
	SC4GameLanguageTable sc4gameLanguageTable;
	bool replacedMaxisPrintFunctionPointers;
};

cRZCOMDllDirector* RZGetCOMDllDirector() {
	static LuaExtensionsDllDirector sDirector;
	return &sDirector;
}