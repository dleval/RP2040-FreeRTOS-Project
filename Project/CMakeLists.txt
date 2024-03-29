
cmake_minimum_required(VERSION 3.14)

# Set app-side debugging "ON" or "OFF"
set(DEBUG_MODE "ON")

# Should we set app-level debugging?
# NOTE Equivalent of `#define DEBUG 1`
if(${DEBUG_MODE})
    add_compile_definitions(DEBUG=1)
    message(STATUS "App-side debugging enabled for ${APP_1_NAME}")
else()
    message(STATUS "App-side debugging disabled for ${APP_1_NAME}")
endif()

# Make project data accessible to compiler
add_compile_definitions(APP_NAME="${APP_NAME}")
add_compile_definitions(APP_VERSION="${APP_VERSION_NUMBER}")
add_compile_definitions(BUILD_NUM=${BUILD_NUMBER})

# Include app header code file(s)
include_directories(${APP_SRC_DIRECTORY}/inc)

# Include app source code file(s)
file(GLOB SOURCES src/*.c)
add_executable(${APP_NAME}
    ${SOURCES}
)


target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Config)

# Link to built libraries
target_link_libraries(${APP_NAME} LINK_PUBLIC
    pico_multicore
    pico_stdlib 
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap3)

target_link_options(${APP_NAME} PRIVATE
    -Xlinker --print-memory-usage
)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(${APP_NAME} 1)
pico_enable_stdio_uart(${APP_NAME} 1)

# Enable extra build products
pico_add_extra_outputs(${APP_NAME})