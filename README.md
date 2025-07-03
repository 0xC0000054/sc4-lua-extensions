# sc4-lua-extensions

This DLL provides a number of functions that allow Lua scripts to do the following.

* Run cheats and pause/resume the game.
* Have better access to SC4's built-in number formatting.
* Get or set the city/mayor names and plop lots.
* Manipulate the in-game camera and take snapshots.

See [null45_lua_extensions.lua](https://github.com/0xC0000054/sc4-lua-extensions/blob/main/dat/null45_lua_extensions.lua) for a list of the provided functions,
and [null45_lua_extensions_tests.lua](https://github.com/0xC0000054/sc4-lua-extensions/blob/main/dat/null45_lua_extensions_tests.lua) for examples of their use.

It also re-enables the Lua error logging that Maxis disabled in the retail version of the game.
Note that due to the way that the game loads plugin Lua scripts, the error messages only give
the line number within the script unless the error occurs within a function.

## Download

The plugin can be downloaded from the Releases tab: https://github.com/0xC0000054/sc4-lua-extensions/releases

## System Requirements

* SimCity 4 version 641
* Windows 10 or later
* [Microsoft Visual C++ 2022 x86 Redistribute](https://aka.ms/vs/17/release/vc_redist.x86.exe)

The plugin may work on Windows 7 or later, but I do not have the ability to test that.

## Installation

1. Close SimCity 4.
2. Copy _SC4LuaExtensions.dll_, and _SC4LuaExtensions.dat_ into the top-level of the Plugins folder in the SimCity 4 installation directory or Documents/SimCity 4 directory.
3. Start SimCity 4.

## Troubleshooting

The plugin should write a _SC4LuaExtensions.log_ file in the same folder as the plugin.    
The log contains status information for the most recent run of the plugin.

# License

This project is licensed under the terms of the GNU Lesser General Public License version 3.0.    
See [LICENSE.txt](LICENSE.txt) for more information.

## 3rd party code

[gzcom-dll](https://github.com/nsgomez/gzcom-dll/tree/master) - MIT License.      
[Windows Implementation Library](https://github.com/microsoft/wil) - MIT License    
[SC4Fix](https://github.com/nsgomez/sc4fix) - MIT License.    
[SafeInt](https://github.com/dcleblanc/SafeInt) - MIT License   

# Source Code

## Prerequisites

* Visual Studio 2022
* `git submodule update --init`

## Building the plugin

* Open the solution in the `src` folder
* Update the post build events to copy the build output to you SimCity 4 application plugins folder.
* Build the solution

## Debugging the plugin

Visual Studio can be configured to launch SimCity 4 on the Debugging page of the project properties.
I configured the debugger to launch the game in a window with the following command line:    
`-intro:off -CPUcount:1 -w -CustomResolution:enabled -r1920x1080x32`

You may need to adjust the resolution for your screen.
