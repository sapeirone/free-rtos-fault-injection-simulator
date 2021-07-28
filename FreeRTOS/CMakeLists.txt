cmake_minimum_required(VERSION 3.10)

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

# set the project name
project(FreeRTOS_Fault_Injection_Simulator C CXX)

include(ExternalProject)

#include(ExternalProject.cmake)

set(sim_install_dir ${CMAKE_CURRENT_BINARY_DIR}/)

if (MSVC)
 ExternalProject_Add(win32-port
        WORKING_DIRECTORY Demo/WIN32-MSVC
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     msbuild WIN32.vcxproj
        )
endif()

if (UNIX)
 ExternalProject_Add_Step(posix-port
         SOURCE_DIR Demo/Posix_GCC
         CONFIGURE_COMMAND ""
         BUILD_COMMAND     make
         INSTALL_COMMAND   make DESTDIR=${sim_install_dir} install
         )
 # ExternalProject_Add_StepTargets(posix-port build)
endif()

# add the executable
# add_executable(test hello.c)

install(DIRECTORY ${sim_install_dir}/ DESTINATION "/")