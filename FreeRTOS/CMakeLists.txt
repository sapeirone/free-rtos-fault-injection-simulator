project(sim C)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)

set(FREERTOS_DIR ".")
set(KERNEL_DIR "./Source")
set(FREERTOS_PLUS_DIR "../FreeRTOS-Plus")

include_directories(${KERNEL_DIR}/include)
include_directories(${FREERTOS_DIR}/Demo/Common/include)
include_directories(${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/Include)

include_directories(${FREERTOS_DIR}/Demo/Posix_GCC/)
include_directories(${KERNEL_DIR}/portable/ThirdParty/GCC/Posix)
include_directories(${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils)

file(GLOB FreeRTOS_src ${KERNEL_DIR}/*.c)
list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/MemMang/heap_3.c)

list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c)
list(APPEND FreeRTOS_src ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c)

add_definitions(-DprojCOVERAGE_TEST=0)
add_definitions(-D_WINDOWS_)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ggdb3 -O0")

file(GLOB common_full ./Demo/Common/Full/*.c)
file(GLOB common_minimal ./Demo/Common/Minimal/*.c)

add_executable(${PROJECT_NAME}
        ./Demo/Posix_GCC/FreeRTOSConfig.h
        ./Demo/Posix_GCC/main.c
        ./Demo/Posix_GCC/main_full.c
        ./Demo/Posix_GCC/main_blinky.c
        ./Demo/Posix_GCC/console.h
        ./Demo/Posix_GCC/console.c
        ./Demo/Posix_GCC/run-time-stats-utils.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/AbortDelay.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/BlockQ.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/blocktim.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/countsem.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/death.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/dynamic.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/EventGroupsDemo.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/flop.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/GenQTest.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/integer.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/IntSemTest.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferAMP.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferDemo.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/PollQ.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/QPeek.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/QueueOverwrite.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/QueueSet.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/QueueSetPolling.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/recmutex.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/semtest.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/StaticAllocation.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferDemo.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferInterrupt.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/TaskNotify.c
        ${FREERTOS_DIR}/Demo/Common/Minimal/TimerDemo.c
        ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
        ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcSnapshotRecorder.c
        ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcStreamingRecorder.c
        ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/streamports/File/trcStreamingPort.c
)

# Link FreeRTOS as a static library
add_library(freertos STATIC ${FreeRTOS_src})
target_link_libraries(${PROJECT_NAME} freertos)
target_link_libraries(${PROJECT_NAME} pthread)
