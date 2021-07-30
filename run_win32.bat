ECHO "Building for Win32 target..."

del CMakeCache.txt
cmake . -G "Visual Studio 16" -A Win32
msbuild.exe ALL_BUILD.vcxproj

ECHO "Done!"

