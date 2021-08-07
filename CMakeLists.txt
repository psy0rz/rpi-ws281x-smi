cmake_minimum_required(VERSION 3.16)
project (rpi_ws281x_smi LANGUAGES C CXX)

include_directories(${CMAKE_JS_INC})

# Include node-addon-api wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})



add_library(${PROJECT_NAME} SHARED${CMAKE_JS_SRC} main.cpp smi/rpi_dma_utils.c smileds.c smileds.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        CXX_STANDARD 20
#        CXX_STANDARD_REQUIRED YES
#        CXX_EXTENSIONS NO
#        )



