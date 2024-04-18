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

set(TSYS_INCLUDE_DIRS "$ENV{TSYS_DIR}")

set(TSYS_LIBRARY_DIR "$ENV{TSYS_DIR}/Tsys/${RELEASE_BUILD_DIR}")

set(UI_TSYS_LIBRARY_DIR "$ENV{UI_TSYS_DIR}/UiTSys/${RELEASE_BUILD_DIR}")

set(UI_RES_LIBRARY_DIR "$ENV{UI_RES_DIR}/UiRes/${RELEASE_BUILD_DIR}")

set(QT_AWESOME_LIBRARY_DIR "$ENV{QT_AWESOME_DIR}/${CMAKE_BUILD_TYPE}")

set(RAPIDJSON_INCLUDE_DIR "$ENV{RAPIDJSON_DIR}")

message("TSYS_INCLUDE_DIRS : ${TSYS_INCLUDE_DIRS}")
message("TSYS_LIBRARY_DIR : ${TSYS_LIBRARY_DIR}")


find_package(Python 3.10 REQUIRED COMPONENTS Development Interpreter)

find_package(Boost 1.77.0 COMPONENTS python310 REQUIRED)


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


