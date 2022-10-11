# Discord Installation Failed Fixer

This `C++` application fixes the common "Discord Installation has failed" issue as well as a grey screen Discord on startup (Discord not working).

## Compiling

Open the `*.sln` file in [`Visual Studio 2022`](https://visualstudio.microsoft.com/downloads) and make sure your compiler supports `C++20`. Also make sure the [`cURLpp`](https://github.com/jpbarrette/curlpp) library is installed e.g. using the package manager [`vcpkg`](https://github.com/microsoft/vcpkg) via `vcpkg install cURLpp:x64-windows`. For the debug build you need dynamic libraries and for the release build static libraries. For the latter, the `vcpkg` command would be `vcpkg install cURLpp:x64-windows-static`. Finally build/run/debug as usual in `Visual Studio`.

## Usage

Run the application and follow the instructions on the command line. A compiled version is provided under the GitHub Releases.

## Credits

Based on [this](https://www.youtube.com/watch?v=14_dlj032Eo) tutorial but programmed/automated by BullyWiiPlaza