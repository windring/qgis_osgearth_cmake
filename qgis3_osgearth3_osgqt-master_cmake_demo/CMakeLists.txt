cmake_minimum_required(VERSION 3.8)

project(TileMapManagerDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(-D_USE_MATH_DEFINES)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

message(STATUS "Find libraries suffixes: ${CMAKE_FIND_LIBRARY_SUFFIXES}")

list(APPEND CMAKE_MODULE_PATH "${${PROJECT_NAME}_SOURCE_DIR}/CMakeModules")

set(OUR_DEV_PACKAGE "C:/dev/vcpkg/installed/x64-windows")
set(ENV{OSGEO4W_ROOT} "C:/Program Files/QGIS 3.22.7/")
set(OSGEO4W_QGIS_SUBDIR "qgis-ltr")
set(Qt5_DIR "C:/Program Files/QGIS 3.22.7/apps/Qt5/lib/cmake/Qt5")
set(OSG_DIR "${OUR_DEV_PACKAGE}")
set(OSGEARTH_DIR "${OUR_DEV_PACKAGE}")
set(GDAL_DIR "${OUR_DEV_PACKAGE}")
# set(GEOS_DIR "${OUR_DEV_PACKAGE}")
# set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL")

find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL Xml REQUIRED)
if (Qt5_FOUND)
    message(STATUS "Qt5 found")
else (Qt5_FOUND)
    message(FATAL_ERROR "Qt5 not found")
endif (Qt5_FOUND)

# find_package(OpenSceneGraph REQUIRED osg osgAnimation osgDB osgFX osgGA osgManipulator osgParticle osgPresentation osgShadow osgSim osgTerrain osgText osgUtil osgViewer osgVolume osgWidget)
# if (OPENSCENEGRAPH_FOUND)
#     message(STATUS "OPENSCENEGRAPH_LIBRARIES:
#         ${OPENSCENEGRAPH_LIBRARIES}")
#     message(STATUS "OPENSCENEGRAPH_INCLUDE_DIR:
#         ${OPENSCENEGRAPH_INCLUDE_DIR}")
# else (OPENSCENEGRAPH_FOUND)
#     message(FATAL_ERROR "OpenSceneGraph not found")
# endif (OPENSCENEGRAPH_FOUND)

find_package(OSG REQUIRED osg osgAnimation osgDB osgFX osgGA osgManipulator osgParticle osgPresentation osgShadow osgSim osgTerrain osgText osgUtil osgViewer osgVolume osgWidget)
if (OSG_FOUND)
    message(STATUS "OSG_LIBRARY:
        ${OSG_LIBRARY}")
    message(STATUS "OSG_INCLUDE_DIR:
        ${OSG_INCLUDE_DIR}")
else (OSG_FOUND)
    message(FATAL_ERROR "OpenSceneGraph not found")
endif (OSG_FOUND)

find_package(OsgEarth REQUIRED)
if (OSGEARTH_FOUND)
    message(STATUS "OSGEARTH_LIBRARY:
        ${OSGEARTH_LIBRARY}")
    message(STATUS "OSGEARTH_INCLUDE_DIR:
        ${OSGEARTH_INCLUDE_DIR}")
else (OSGEARTH_FOUND)
    message(FATAL_ERROR "osgearth not found:")
endif (OSGEARTH_FOUND)

find_package(QGIS REQUIRED)
if (QGIS_FOUND)
    message(STATUS "QGIS_INCLUDE_DIR:
        ${QGIS_INCLUDE_DIR}")
else (QGIS_FOUND)
    message(FATAL_ERROR "QGIS not found!")
endif (QGIS_FOUND)

find_package(OpenThreads REQUIRED)
if (OPENTHREADS_FOUND)
    message(STATUS "OPENTHREADS_LIBRARY:
        ${OPENTHREADS_LIBRARY}")
    message(STATUS "OPENTHREADS_INCLUDE_DIR:
        ${OPENTHREADS_INCLUDE_DIR}")
else (OPENTHREADS_FOUND)
    message(FATAL_ERROR "openthreads not found")
endif (OPENTHREADS_FOUND)

# find_package(GEOS CONFIG REQUIRED)
# if (GEOS_FOUND)
#     message(STATUS "GEOS_LIBRARY:
#     ${GEOS_LIBRARY}")
#     message(STATUS "GEOS_INCLUDE_DIR:
#     ${GEOS_INCLUDE_DIR}")
# else (GEOS_FOUND)
#     message (FATAL_ERROR "GEOS dependency was not found!")
# endif (GEOS_FOUND)

find_package(GDAL REQUIRED)
if (GDAL_FOUND)
    message(STATUS "GDAL_LIBRARY:
    ${GDAL_LIBRARY}")
    message(STATUS "GDAL_INCLUDE_DIR:
    ${GDAL_INCLUDE_DIR}")
else (GDAL_FOUND)
    message (FATAL_ERROR "GDAL dependency was not found!")
endif (GDAL_FOUND)

# find_package(CURL REQUIRED)
# if (CURL_FOUND)
#     message(STATUS "CURL_LIBRARIES:
#     ${CURL_LIBRARIES}")
#     message(STATUS "CURL_INCLUDE_DIRS:
#     ${CURL_INCLUDE_DIRS}")
# else (CURL_FOUND)
#     message (FATAL_ERROR "CURL dependency was not found!")
# endif (CURL_FOUND)

# find_package(Threads REQUIRED)
# if (Threads_FOUND)
#     message(STATUS "CMAKE_THREAD_LIBS_INIT:
#     ${CMAKE_THREAD_LIBS_INIT}")
# else (Threads_FOUND)
#     message (FATAL_ERROR "Threads dependency was not found!")
# endif (Threads_FOUND)

# set(OpenGL_GL_PREFERENCE GLVND)
# find_package(OpenGL REQUIRED)

find_package(PostgreSQL REQUIRED)
if (PostgreSQL_FOUND)
    message(STATUS "PostgreSQL version:
        ${PostgreSQL_VERSION_STRING}")
    message(STATUS "PostgreSQL_LIBRARIES:
        ${PostgreSQL_LIBRARIES}")
    message(STATUS "PostgreSQL_INCLUDE_DIR:
        ${PostgreSQL_INCLUDE_DIR}")
else (PostgreSQL_FOUND)
    message(STATUS "PostgreSQL not found")
endif (PostgreSQL_FOUND)

include_directories(
        ${OSGEARTH_INCLUDE_DIR}
        # ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OSG_INCLUDE_DIR}
        ${QGIS_INCLUDE_DIR}
        osgQOpenGL
        ${QT_INCLUDE_DIR}
        # ${GEOS_INCLUDE_DIR}
        ${GDAL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIR}
)

aux_source_directory(osgQOpenGL OSGQOPENGL_SRC)
aux_source_directory(MultiLayerTileMap MULTILAYERTILEMAP_SRC)

link_directories("${OUR_DEV_PACKAGE}/lib/"
    "${OUR_DEV_PACKAGE}/lib64/"
    "${OUR_DEV_PACKAGE}/lib/osgPlugins-3.6.5/"
    "${OUR_DEV_PACKAGE}/lib64/osgPlugins-3.6.5/"
    )

add_executable(${PROJECT_NAME}
        main.cpp
        ${OSGQOPENGL_SRC})

if(UNIX AND NOT APPLE)
    # linux
    set(LINUX_ADDITION_LINK
        ${CURL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
        ${PostgreSQL_LIBRARIES}
        # -losgdb_osg
        # -losgdb_earth
        # -losgdb_osgearth_engine_rex
        # -losgdb_zip
        # -losgdb_gdal
        -losgEarth
        -lproj
        -lgeos_c
        -lgeos
        -lexpat
        -lsqlite3
        -lz
        -lpthread
        -lGL
        -lpng16
        -ltiff
        -lcurl-gnutls
        -lgeos-3.9.1
        -ljson-c
        -lwebp
        -lnetcdf
        -lhdf5_serial
        -lgif
        -lgeotiff
        -ljpeg
        -lxml2
        -lz
        -lzstd
        -llzma
        -lfontconfig
        -lfreetype
        -lbrotlidec
        -lbz2
        -lhdf5_serial_hl
        -ltinyxml
        -lopenjp2
        -lhdf5
        -losg
        -losgDB
        -losgGA
        -losgTerrain
        -losgText
        -losgUI
        -losgUtil
        -losgViewer
        # OpenGL::GL
        # -lfontconfig
        # -lexpat
        # -lfreetype
        # -lz
        # -lpng16
        # -lbrotlidec
        # -lbz2
        # -lsqlite3
        # -ltiff
        # -lgeos-3.9.1
        # -ljson-c
        # -lwebp
        # -lnetcdf
        # -lhdf5_serial
        # -lgif
        # -lgeotiff
        # -ljpeg
        # -lxml2
        # -lz
        # -lzstd
        # -llzma
        # -lhdf5_serial_hl
        # -ltinyxml
    )
endif()

set(LINK_LIBS
        ${OSGEARTH_LIBRARY}
        ${OSG_LIBRARY}
        ${OSGGA_LIBRARY}
        ${OSGUTIL_LIBRARY}
        ${OSGDB_LIBRARY}
        ${OSGTEXT_LIBRARY}
        ${OSGWIDGET_LIBRARY}
        ${OSGTERRAIN_LIBRARY}
        ${OSGFX_LIBRARY}
        ${OSGVIEWER_LIBRARY}
        ${OSGVOLUME_LIBRARY}
        ${OSGMANIPULATOR_LIBRARY}
        ${OSGANIMATION_LIBRARY}
        ${OSGPARTICLE_LIBRARY}
        ${OSGSHADOW_LIBRARY}
        ${OSGPRESENTATION_LIBRARY}
        ${OSGSIM_LIBRARY}
        ${OPENTHREADS_LIBRARY}
        ${QGIS_CORE_LIBRARY}
        ${QGIS_GUI_LIBRARY}
        ${GDAL_LIBRARY}
        Qt5::Core
        Qt5::Widgets
        Qt5::Xml
        Qt5::OpenGL
        Qt5::Gui
        # ${GEOS_LIBRARY}
        ${LINUX_ADDITION_LINK})

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

get_target_property(CURRENT_LINK_LIBRARIES ${PROJECT_NAME} LINK_LIBRARIES)
message(STATUS "CURRENT_LINK_LIBRARIES: ${CURRENT_LINK_LIBRARIES}")

if (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    # vscode
    set(OUR_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

else()
    # vs, clion, etc
    set(OUR_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

message(STATUS "Copy assets file to ${OUR_OUT_DIR}")
add_custom_target(
        copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data "${OUR_OUT_DIR}/data"
)

add_dependencies(${PROJECT_NAME} copy_assets)
if(WIN32)
    set(QT_PLATFORMS_DIR "$ENV{OSGEO4W_ROOT}/apps/Qt5/plugins/platforms")
    message(STATUS "Copy platforms file to ${OUR_OUT_DIR}")
    add_custom_target(
            copy_platforms
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT_PLATFORMS_DIR} "${OUR_OUT_DIR}/platforms"
    )
    add_dependencies(${PROJECT_NAME} copy_platforms)
endif(WIN32)

enable_testing()
add_subdirectory(tests)
message(STATUS "tests: ${TILEMAP_TESTS_SRC}")

add_executable(TestMapLayerManager
        ${OSGQOPENGL_SRC}
        ${MULTILAYERTILEMAP_SRC}
        tests/TestMapLayerManager.cpp)
target_link_libraries(TestMapLayerManager ${LINK_LIBS})
add_test(NAME TestMapLayerManager COMMAND TestMapLayerManager)

add_executable(TestGroundVehicle tests/TestGroundVehicle.cpp ${OSGQOPENGL_SRC} ${MULTILAYERTILEMAP_SRC})
target_link_libraries(TestGroundVehicle ${LINK_LIBS})
add_dependencies(TestGroundVehicle copy_assets)
add_test(NAME TestGroundVehicle COMMAND TestGroundVehicle)

add_executable(TestPrecipitationEffect tests/TestPrecipitationEffect.cpp)
target_link_libraries(TestPrecipitationEffect ${LINK_LIBS})
add_test(NAME TestPrecipitationEffect COMMAND TestPrecipitationEffect)
