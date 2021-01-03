# README

This is still pretty rough I haven't had that much time to work on it. An example of how to use the compiler API is in the ScriptCompileTester project.
The decompiler is still mostly unusable.

To build you need to recursively clone submodules and also place a copy of [win_flex_bison](https://github.com/lexxmark/winflexbison/releases/) such that `$(SolutionDir)win_flex_bison\win_bison.exe` and `$(SolutionDir)win_flex_bison\win_flex.exe` are valid paths.