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

// Portions of this file have been adapted from:
/*
   Project: SC4Fix Patches for SimCity 4
   File: version.cpp

   Copyright (c) 2015 Nelson Gomez (simmaster07)

   Licensed under the MIT License. A copy of the License is available in
   LICENSE or at:

	   http://opensource.org/licenses/MIT

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "SC4VersionDetection.h"
#include <Windows.h>
#include "wil/resource.h"
#include "wil/win32_helpers.h"

namespace
{
	uint64_t GetAssemblyVersion(HMODULE hModule)
	{
		wil::unique_cotaskmem_string szVersionFile;

		if (SUCCEEDED((wil::GetModuleFileNameW<wil::unique_cotaskmem_string, 128>(hModule, szVersionFile))))
		{
			// http://stackoverflow.com/a/940743
			DWORD verHandle = 0;
			UINT  size = 0;
			DWORD verSize = GetFileVersionInfoSizeW(szVersionFile.get(), &verHandle);

			if (verSize > 0)
			{
				auto verData = wil::make_unique_cotaskmem_nothrow<BYTE[]>(verSize);

				if (verData)
				{
					LPBYTE lpBuffer = nullptr;

					if (GetFileVersionInfoW(szVersionFile.get(), 0, verSize, verData.get())
						&& VerQueryValueW(verData.get(), L"\\", reinterpret_cast<LPVOID*>(&lpBuffer), &size)
						&& size > 0)
					{
						VS_FIXEDFILEINFO* verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
						if (verInfo->dwSignature == 0xfeef04bd)
						{
							uint64_t qwValue = static_cast<uint64_t>(verInfo->dwFileVersionMS) << 32;
							qwValue |= verInfo->dwFileVersionLS;

							return qwValue;
						}
					}
				}
			}
		}

		return 0;
	}

	uint16_t DetermineGameVersion()
	{
		uint64_t qwFileVersion = GetAssemblyVersion(nullptr);
		uint16_t wMajorVer = (qwFileVersion >> 48) & 0xFFFF;
		uint16_t wMinorVer = (qwFileVersion >> 32) & 0xFFFF;
		uint16_t wRevision = (qwFileVersion >> 16) & 0xFFFF;
		uint16_t wBuildNum = qwFileVersion & 0xFFFF;

		uint16_t nGameVersion = 0;

		// 1.1.x.x
		if (qwFileVersion > 0 && wMajorVer == 1 && wMinorVer == 1)
		{
			nGameVersion = wRevision;
		}
		else
		{
			nGameVersion = 0;
		}

		// Fall back to a less accurate detection mechanism
		if (nGameVersion == 0)
		{
			uint8_t uSentinel = *(uint8_t*)0x6E5000;

			switch (uSentinel)
			{
			case 0x8B:
				nGameVersion = 610; // Can't distinguish from 613
				break;
			case 0xFF:
				nGameVersion = 638;
				break;
			case 0x24:
				nGameVersion = 640;
				break;
			case 0x0F:
				nGameVersion = 641;
				break;
			default:
				nGameVersion = 0;
				break;
			}
		}

		return nGameVersion;
	}
}

uint16_t SC4VersionDetection::GetGameVersion()
{
	static uint16_t gameVersion = DetermineGameVersion();

	return gameVersion;
}

bool SC4VersionDetection::IsVersion641()
{
	static bool isVersion641 = GetGameVersion() == 641;

	return isVersion641;
}
