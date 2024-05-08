cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHsc")


project(GexMaya)


add_definitions("/DQT_NO_KEYWORDS")
add_definitions("/DGEX_UI_API_IMPORT")
#add_compile_options(/Zc:__cplusplus)
#add_compile_options(/permissive-)


set(MAYA_INCLUDE_DIRS "$ENV{DEVKIT_LOCATION}/include")

set(MAYA_LIBRARY_DIRS "$ENV{DEVKIT_LOCATION}/lib")

set(RELEASE_BUILD_DIR "cmake-build-release")
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(RELEASE_BUILD_DIR "cmake-build-debug")
endif()

set(GEX_INCLUDE_DIRS "$ENV{GEX_ROOT}")

set(GEX_LIBRARY_DIRS "$ENV{GEX_ROOT}/Gex/${RELEASE_BUILD_DIR}")

set(GEX_UI_LIBRARY_DIRS "$ENV{GEX_ROOT}/Gex/${RELEASE_BUILD_DIR}/ui")

set(GEX_PYTHON_DIR "$ENV{GEX_ROOT}/Gex/${RELEASE_BUILD_DIR}/python/Gex")

set(TSYS_INCLUDE_DIRS "$ENV{TSYS_DIR}")

set(TSYS_LIBRARY_DIR "$ENV{TSYS_DIR}/Tsys/${RELEASE_BUILD_DIR}")

set(UI_TSYS_LIBRARY_DIR "$ENV{UI_TSYS_DIR}/UiTSys/${RELEASE_BUILD_DIR}")

set(UI_RES_LIBRARY_DIR "$ENV{UI_RES_DIR}/UiRes/${RELEASE_BUILD_DIR}")

set(QT_AWESOME_LIBRARY_DIR "$ENV{QT_AWESOME_DIR}/${CMAKE_BUILD_TYPE}")

set(RAPIDJSON_INCLUDE_DIR "$ENV{RAPIDJSON_DIR}")


set(BOOST_ROOT ENV{BOOST_ROOT})

find_package(Python 3.11 REQUIRED COMPONENTS Development Interpreter)

find_package(Boost 1.82.0 COMPONENTS python311 REQUIRED)


# Find Qt package -------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets Core Gui Svg SvgWidgets REQUIRED)
qt_standard_project_setup()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


include_directories(
        ${CMAKE_SOURCE_DIR}
)

add_subdirectory(MayaPlugin)

add_subdirectory(PythonWrapper)


add_dependencies(
        GexMayaPython
        GexMaya
)


add_custom_target(
        GexMayaPythonBuild
        ALL
        DEPENDS
        GexMaya
        GexMayaPython
)

add_dependencies(
        GexMayaPythonBuild
        GexMayaPython
        GexMaya
)



set(PACKAGED_ENV "{'GEX_RESOURCES_PATH':'resources', 'PYTHON_PATH': 'scripts'}")


add_custom_command(
        TARGET
        GexMayaPythonBuild
        COMMAND
        python ${CMAKE_SOURCE_DIR}/build/packageModule.py

        --name GexMaya

        --libraries ${GEX_LIBRARY_DIRS}/Gex.dll
        ${TSYS_LIBRARY_DIR}/Tsys.dll
        ${GEX_UI_LIBRARY_DIRS}/Gex_ui.dll
        ${QT_AWESOME_LIBRARY_DIR}/QtAwesome.dll

        --plugins ${CMAKE_BINARY_DIR}/MayaPlugin/GexMaya.mll

        --scripts ${GEX_PYTHON_DIR}

        --custom-dir ${CMAKE_SOURCE_DIR}/MayaPlugin/resources

        --description GexMaya@0.0@Module@env=${PACKAGED_ENV}

        ${CMAKE_BINARY_DIR}/Module

        COMMAND
        python $ENV{BUILD_UTILS_DIR}/BuildUtils/dependencies.py
        --depth 2
        --search-directories ${GEX_LIBRARY_DIRS} ${TSYS_LIBRARY_DIR}
        ${GEX_UI_LIBRARY_DIRS} ${QT_AWESOME_LIBRARY_DIR}
        ${UI_TSYS_LIBRARY_DIR} ${UI_RES_LIBRARY_DIR}

        --exclude-current

        ${CMAKE_BINARY_DIR}/MayaPlugin/GexMaya.mll
        ${CMAKE_BINARY_DIR}/Module/GexMaya/Module/libraries

        COMMAND
        python $ENV{BUILD_UTILS_DIR}/BuildUtils/build_package.py
        --directory ${CMAKE_BINARY_DIR}/Module/GexMaya/Module/scripts
        --name GexMaya --search_directories ${CMAKE_SOURCE_DIR} ${MAYA_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS}
        ${Python_LIBRARY_DIRS} ${GEX_LIBRARY_DIRS} ${TSYS_LIBRARY_DIR} ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        --library ${CMAKE_BINARY_DIR}/GexMayaPython.dll
        --dependencies Gex.dll --template-init ${CMAKE_SOURCE_DIR}/PythonWrapper/resources/__init__.py

        DEPENDS
        GexMaya GexMayaPython
)


