echo off
Setlocal EnableDelayedExpansion

echo Free RTOS injection simulator: building for Win32 target...

@rem Despite /r "Processes search strings as regular expressions" findstr ignores the regex

set generator=null
for /f "delims=="   %%l in ('cmake --help ^| findstr /r "Visual Studio \d+ \d{4}\s+(?![arch])="') do (
  echo "%%l" | find "arch">nul
  if errorlevel 1 (
    if !generator!==null (
      set generator=%%l
    ) else (
      if "%l:~0,1%"=="*" (
        set generator=%%l
      )
    )
  )
)

if "%generator%"=="" (
    echo No suitable cmake generator found! Aborting...
    exit /b 1
)

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

del /s CMakeCache.txt > nul 2>&1

echo Compiling...

cmake . -G "%generator%" -A Win32 > cmake_build.log 2>&1
msbuild.exe ALL_BUILD.vcxproj > msbuild.log 2>&1

set BUILD_STATUS=%ERRORLEVEL%
if %BUILD_STATUS%==0 (
    move build\Debug\sim.exe sim.exe > nul
    echo Done! You can run the simulator with ./sim.exe
    exit /b 0
) 

echo Compilation error! Check cmake_build.log and msbuild.log
