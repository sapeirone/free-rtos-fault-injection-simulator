ECHO "Building for Win32 target..."

del /s CMakeCache.txt >nul
cmake . -G "Visual Studio 17" -A Win32
msbuild.exe ALL_BUILD.vcxproj

copy build\Debug\sim.exe sim.exe

ECHO "Done!"

