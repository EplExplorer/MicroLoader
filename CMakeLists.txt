cmake_minimum_required(VERSION 3.20)
project(MicroLoader VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE KRNLN_SOURCE "ELoader/krnln/*.cpp")

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")

include_directories(ELoader/bdwgc/include)

add_executable(MicroLoader
        ELoader/EArray.cpp
        ELoader/EContext.cpp
        ELoader/EHelpFunc.cpp
        ELoader/ELoader.cpp
        ELoader/ENotifySys.cpp
        ELoader/EGC.cpp
        ELoader/logger/easylogging++.cc
        ${KRNLN_SOURCE})

add_custom_command(TARGET MicroLoader POST_BUILD
        COMMAND python ${CMAKE_SOURCE_DIR}/tools/inject.py $<TARGET_FILE:MicroLoader>
        VERBATIM
)