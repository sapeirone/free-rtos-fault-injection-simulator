echo off

echo Free RTOS injection simulator: building for Win32 target...

@rem Despite /r "Processes search strings as regular expressions" findstr ignores the regex
for /f "delims=="   %%l in ('cmake --help ^| findstr /r "Visual Studio \d+ \d{4}\s+(?![arch])="') do (
  echo "%%l" | find "arch">nul
  if errorlevel 1 (
      set generator=%%l
      goto :build
  )
)

if "%generator%"=="" (
    echo No suitable cmake generator found! Aborting...
    exit /b 1
)

:build

set generator=%generator:~2%

@rem i don't know why the following code works and i'm too afraid to ask
@rem (source: https://www.dostips.com/DtTipsStringManipulation.php#Snippets.TrimRightFOR)
set generator=%generator%##
set generator=%generator:                ##=##%
set generator=%generator:        ##=##%
set generator=%generator:    ##=##%
set generator=%generator:  ##=##%
set generator=%generator: ##=##%
set generator=%generator:##=%

echo Selected generator is "%generator%"

del /s CMakeCache.txt >nul

echo Compiling...

cmake . -G "%generator%" -A Win32 > cmake_build.log
msbuild.exe ALL_BUILD.vcxproj > msbuild.log

set BUILD_STATUS=%ERRORLEVEL%
if %BUILD_STATUS%==0 (
    move build\Debug\sim.exe sim.exe > nul
    echo Done! You can run the simulator with ./sim.exe
    exit /b 0
) 

echo Compilation error! Check cmake_build.log and msbuild.log
