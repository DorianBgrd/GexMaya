cmake_minimum_required(VERSION 3.24)

project(GexMaya)


add_library(
        GexMaya
        SHARED
        include/MayaPlugin.h
        sources/MayaPlugin.cpp
        include/GraphView.h
        sources/GraphView.cpp
        include/MayaTypeRegistry.h
        sources/MayaTypeRegistry.cpp
        include/Nodes.h
        sources/Nodes.cpp
        include/Plugin.h
        sources/Plugin.cpp
        include/Types.h
        sources/Types.cpp
        include/Data.h
        sources/Data.cpp
)


target_include_directories(
        GexMaya
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

        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Gui_INCLUDE_DIRS}
        ${Qt6Svg_INCLUDE_DIRS}
        ${Qt6SvgWidgets_INCLUDE_DIRS}
)


target_link_directories(
        GexMaya
        PUBLIC

        ${MAYA_LIBRARY_DIRS}
        ${Boost_LIBRARY_DIRS}
        ${Python_LIBRARY_DIRS}

        ${GEX_LIBRARY_DIRS}
        ${GEX_UI_LIBRARY_DIRS}

        ${TSYS_LIBRARY_DIR}
        ${UI_TSYS_LIBRARY_DIR}
        ${UI_RES_LIBRARY_DIR}
        ${QT_AWESOME_LIBRARY_DIR}
)


target_link_libraries(
        GexMaya
        PUBLIC

        Foundation
        OpenMaya
        OpenMayaAnim
        OpenMayaUI

        Gex
        Tsys
        UiTSys
        UiRes
        Gex_ui

        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
        Qt6::SvgWidgets
        QtAwesome
)


#add_custom_command(
#        TARGET
#        GexMaya
#
#        POST_BUILD
#        COMMAND
#        python $ENV{BUILD_UTILS_DIR}/BuildUtils/build_package.py --directory ${CMAKE_CURRENT_BINARY_DIR}
#        --name GexMaya
#
#        --search_directories ${MAYA_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} ${Python_LIBRARY_DIRS}
#        ${GEX_LIBRARY_DIRS} ${TSYS_LIBRARY_DIR} ${UI_TSYS_LIBRARY_DIR} ${UI_RES_LIBRARY_DIR}
#        ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${TSYS_LIBRARY_DIR} ${GEX_UI_LIBRARY_DIRS}
#        ${QT_AWESOME_LIBRARY_DIR}
#
#        --dependencies Gex.dll Tsys.dll MayaPlugin.dll Gex_ui.dll UiTSys.dll UiRes.dll QtAwesome.dll
#
#        --library ${CMAKE_BINARY_DIR}/GexMaya.dll
#)


add_custom_command(
        TARGET
        GexMaya

        POST_BUILD
        COMMAND
        python ${CMAKE_SOURCE_DIR}/copy.py ${CMAKE_BINARY_DIR}/GexMaya.dll
        ${CMAKE_CURRENT_BINARY_DIR}/GexMaya.mll
)


