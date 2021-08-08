cmake_minimum_required(VERSION 3.16)
project(rpi_ws281x_smi LANGUAGES C CXX)

if (EXISTS "/proc/device-tree/model")
    file(READ /proc/device-tree/model PI_MODEL_STR)
    MESSAGE("rpi-ws281x-smi: Raspberry device string: ${PI_MODEL_STR}")

    # look in rpi_dma_utils.h for more info!
    if (${PI_MODEL_STR} MATCHES "Pi Model" OR ${PI_MODEL_STR} MATCHES "Pi Zero")
        MESSAGE("rpi-ws281x-smi: Configuring as Pi zero or Pi 1")
        add_definitions(-DPHYS_REG_BASE=0x20000000)
    elseif (${PI_MODEL_STR} MATCHES "Pi 2" OR ${PI_MODEL_STR} MATCHES "Pi 3")
        MESSAGE("rpi-ws281x-smi: Configuring as Pi 2 or Pi 3")
        add_definitions(-DPHYS_REG_BASE=0x3F000000)
    elseif (${PI_MODEL_STR} MATCHES "Pi 4")
        MESSAGE("rpi-ws281x-smi: Configuring as Pi 4")
        add_definitions(-DPHYS_REG_BASE=0xFE000000)
    else()
        MESSAGE(FATAL_ERROR "rpi-ws281x-smi: Unknown rapsberry model string: ${PI_MODEL_STR}")
    endif ()

    add_library(${PROJECT_NAME} SHARED${CMAKE_JS_SRC} main.cpp smi/rpi_dma_utils.c smileds.c smileds.h)
else ()
    MESSAGE(WARNING "rpi-ws281x-smi: NO RASPBERRY DETECTED. BUILDING DUMMY MODULE!")
    add_library(${PROJECT_NAME} SHARED${CMAKE_JS_SRC} main.cpp)
endif ()

include_directories(${CMAKE_JS_INC})

# Include node-addon-api wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})


target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(${PROJECT_NAME} PRIVATE -Werror)

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        CXX_STANDARD 20
#        CXX_STANDARD_REQUIRED YES
#        CXX_EXTENSIONS NO
#        )



