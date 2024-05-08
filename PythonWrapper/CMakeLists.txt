cmake_minimum_required(VERSION 3.24)

project(GexMayaPython)


add_library(
        GexMayaPython
        SHARED
        sources/module.cpp
)


target_include_directories(
        GexMayaPython
        PUBLIC
        ${GEX_INCLUDE_DIRS}
        ${GEX_INCLUDE_DIRS}/Gex
        ${MAYA_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${Python_INCLUDE_DIRS}

        $ENV{RAPIDJSON_DIR}
        $ENV{SAFEPTRS_DIR}
        $ENV{TSYS_DIR}
        $ENV{UI_TSYS_DIR}
        $ENV{UI_RES_DIR}
        $ENV{QT_AWESOME_DIR}
)


target_link_directories(
        GexMayaPython
        PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${MAYA_LIBRARY_DIRS}
        ${Boost_LIBRARY_DIRS}
        ${Python_LIBRARY_DIRS}

        ${GEX_LIBRARY_DIRS}
        ${TSYS_LIBRARY_DIR}
        ${CMAKE_SOURCE_DIR}/MayaPlugin
)


target_link_libraries(
        GexMayaPython
        PUBLIC

        GexMaya
        Foundation

        ${Boost_LIBRARIES}
        ${Python_LIBRARIES}
)
