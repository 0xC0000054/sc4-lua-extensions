# For DLL Developers

## Exposing Functions To Lua

SC4 provides two different mechanisms for exposing native functions to Lua.

## The Low-Level API

The low-level API provided by the game's cISCLua interface.
This API is used by SC4's game table functions (e.g. `game.trend_slope`), but DLLs can define their own uniquely named table
for their native functions.

The API uses a native function format that is very similar to what is described in the[Defining C Functions](https://www.lua.org/manual/5.0/manual.html#3.16)
section of the Lua 5.0 Programming Manual, the main difference being that you are interacting with the Lua API through the game's cISCLua interface.

Native functions are typically registered with Lua using the `SCLuaUtil::RegisterLuaFunction` method.

## The High-Level API

The high-level API is provided by the cISC4GameDataRegistry interface.
This interface allows Lua callbacks to be part of a C++ class and uses cIGZCommandParameterSet to hide the low-level Lua parameter handling code.

The cISC4GameDataRegistry are grouped into child tables under the _sc4game_ table that act as categories, e.g. `sc4game.automata.get_source_building_count`.

## GZCOM Classes

The DLL provides the following GZCOM classes for other DLLs to use.

### cIPackageScriptCompilationCallbackServer

This class allows a DLL to receive callbacks when the game successfully compiles a package Lua script.

The `AnyScriptCompiledCallback` will be called for every package script the game loads, it is intended for debug logging.

The `TargetScriptCompiledCallback` will be called after the specified target script was successfully compiled.
This callback is intended for the special case where a DLL exposes native functions to Lua functions that need to be initialized
before dependent scripts with higher package numbers are compiled, this is typically only necessary for DLLs that provide native
functions that dependent scripts may call to initialize compile-time global variables.

An example of this is the `dbpf.get_exemplar_property_value` function provided by this DLL. Some scripts that depend on this DLL 
call it to initialize global variables to values read from specific exemplar properties.
See [null45_lua_extensions_tests.lua](https://github.com/0xC0000054/sc4-lua-extensions/blob/main/dat/null45_lua_extensions_tests.lua).
 