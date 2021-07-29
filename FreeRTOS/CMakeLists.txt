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

include_directories(./Demo/Posix_GCC/)
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

add_executable(${PROJECT_NAME}
        ./Demo/Posix_GCC/FreeRTOSConfig.h
        ./Demo/Posix_GCC/main.c
        ./Demo/Posix_GCC/main_full.c
        ./Demo/Posix_GCC/console.h
        ./Demo/Posix_GCC/console.c
)

# Link FreeRTOS as a static library
add_library(freertos STATIC ${FreeRTOS_src})
target_link_libraries(${PROJECT_NAME} freertos)
target_link_libraries(${PROJECT_NAME} pthread)
