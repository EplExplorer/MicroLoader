cmake_minimum_required(VERSION 3.20)
project(MicroLoader VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(MicroLoader
        ELoader/EArray.cpp
        ELoader/EContext.cpp
        ELoader/EHelpFunc.cpp
        ELoader/ELoader.cpp
        ELoader/ENotifySys.cpp
        ELoader/logger/easylogging++.cc)

add_custom_command(TARGET MicroLoader POST_BUILD
        COMMAND python ${CMAKE_SOURCE_DIR}/tools/inject.py $<TARGET_FILE:MicroLoader>
        VERBATIM
)