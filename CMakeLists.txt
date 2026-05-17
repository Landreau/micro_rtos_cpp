cmake_minimum_required(VERSION 3.10)
project(micro_rtos CXX)

# Configuration du compilateur
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Sources du kernel
set(KERNEL_SOURCES
    kernel/task.cpp
    kernel/scheduler.cpp
    kernel/rtos.cpp
)

find_package(Threads REQUIRED)

add_executable(micro_rtos main.cpp ${KERNEL_SOURCES})
target_link_libraries(micro_rtos Threads::Threads)

# Options de compilation
if(MSVC)
    target_compile_options(micro_rtos PRIVATE /W4)
else()
    target_compile_options(micro_rtos PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sensor_demo main_sensor_demo.cpp ${KERNEL_SOURCES})
target_link_libraries(sensor_demo Threads::Threads)

if(MSVC)
    target_compile_options(sensor_demo PRIVATE /W4)
else()
    target_compile_options(sensor_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

