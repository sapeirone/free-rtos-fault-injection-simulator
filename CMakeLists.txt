project(sim C ASM)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE ON)

if (WIN32)
    # probably useless:
    set(CMAKE_GENERATOR_PLATFORM Win32 CACHE STRING "" FORCE)
    add_definitions(-DWIN32)
else()
    add_definitions(-DPOSIX)
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)

set(FREERTOS_DIR "./FreeRTOS/")
set(KERNEL_DIR "./FreeRTOS/Source")
set(FREERTOS_PLUS_DIR "./FreeRTOS-Plus")
set(SIMULATOR_DIR "./simulator")

include_directories(${KERNEL_DIR}/include)
include_directories(${FREERTOS_DIR}/Demo/Common/include)

include_directories(${SIMULATOR_DIR})
include_directories(${SIMULATOR_DIR}/injection/include)

# injector specific import
include_directories(${KERNEL_DIR}/injector/include)

# unix-specific includes
if (UNIX)
    include_directories(${SIMULATOR_DIR}/Posix)
    include_directories(${KERNEL_DIR}/portable/ThirdParty/GCC/Posix)
    include_directories(${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils)
elseif (WIN32)
    include_directories(${SIMULATOR_DIR}/Win32)
    include_directories(${KERNEL_DIR}/portable/MSVC-MingW/)
endif()

file(GLOB FreeRTOS_src ${KERNEL_DIR}/*.c)
list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/MemMang/heap_5.c)
list(APPEND FreeRTOS_src ${SIMULATOR_DIR}/injection/common.c)

# unix-specific kernel code
if (UNIX) 
    list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c)
    list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c)
elseif (WIN32)
    list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/MSVC-MingW/port.c)
endif()

if (UNIX)
    # unix-specific compiler parameters
    add_definitions(-DprojCOVERAGE_TEST=0)
    add_definitions(-D_WINDOWS_) # ???

    # -ggdb3 produces extra debugging information
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ggdb3 -O0")
elseif (WIN32)
    # nothing
endif()

# platform-independent source code
file(GLOB sources ${SIMULATOR_DIR}/main.c)
list(APPEND sources ${SIMULATOR_DIR}/main_blinky.c)
list(APPEND sources ${SIMULATOR_DIR}/loggingUtils.c)
list(APPEND sources ${SIMULATOR_DIR}/benchmark/benchmark.c)
list(APPEND sources ${SIMULATOR_DIR}/injection/injection.c)

if (UNIX) 
    # posix-dependent source code
    list(APPEND sources ${SIMULATOR_DIR}/Posix/run-time-stats-utils.c)
    list(APPEND sources ${SIMULATOR_DIR}/Posix/fork.c)
    list(APPEND sources ${SIMULATOR_DIR}/Posix/sleep.c)
    list(APPEND sources ${SIMULATOR_DIR}/Posix/thread.c)
elseif (WIN32)
    # win32-dependent source code
	list(APPEND sources ${SIMULATOR_DIR}/Win32/Run-time-stats-utils.c)
    list(APPEND sources ${SIMULATOR_DIR}/Win32/fork.c)
    list(APPEND sources ${SIMULATOR_DIR}/Win32/sleep.c)
    list(APPEND sources ${SIMULATOR_DIR}/Win32/thread.c)
endif()

add_executable(${PROJECT_NAME} ${sources})

# Link FreeRTOS as a static library
add_library(freertos STATIC ${FreeRTOS_src})
target_link_libraries(${PROJECT_NAME} freertos)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} pthread rt)
endif()
