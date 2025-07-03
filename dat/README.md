# DAT and Lua files

This folder contains the DAT and Lua files used in the plugin.
The Lua files are extracted from the DAT so that they can be shown with GitHub's Lua syntax highlighting.

## null45_lua_extensions.lua

This file contains the function stubs that the DLL will replace with a native C++ function. The functions
are in a Lua tables whose names are known to the DLL.

## null45_lua_extensions_tests.lua

This file contains functions that demonstrate the use of the new Lua functions.
Each function returns the result as a string.

In development builds, the DLL will use theese function to test the corresponding native C++ function.