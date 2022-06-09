cmake_minimum_required(VERSION 3.8)

project(TileMapManagerDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_BUILD_TYPE Release)

add_definitions(-D_USE_MATH_DEFINES)

message(STATUS "Find libraries suffixes: ${CMAKE_FIND_LIBRARY_SUFFIXES}")

list(APPEND CMAKE_MODULE_PATH "${${PROJECT_NAME}_SOURCE_DIR}/CMakeModules")

set(QT5PLUGINS "G:/program/osgeo4w/apps/Qt5/plugins")
set(ENV{OSGEO4W_ROOT} "G:/program/osgeo4w")
set(OSGEO4W_QGIS_SUBDIR "qgis-ltr-dev")
set(Qt5_DIR "G:/program/osgeo4w/apps/Qt5/lib/cmake/Qt5")
set(OSG_DIR "G:/modules/osgearth210vs2015/OpenSceneGraph")
set(OSGEARTH_DIR "G:/modules/osgearth210vs2015/OpenSceneGraph")
# set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL")

find_package(Qt5 COMPONENTS Core Widgets Xml Gui OpenGL REQUIRED)
if (Qt5_FOUND)
    message(STATUS "Qt5 found (for automoc etc)")
else (Qt5_FOUND)
    message(FATAL_ERROR "Qt5 not found")
endif (Qt5_FOUND)

find_package(OpenSceneGraph REQUIRED osg osgDB osgGA osgViewer osgUtil osgText osgWidget)
if (OPENSCENEGRAPH_FOUND)
    message(STATUS "OPENSCENEGRAPH_LIBRARIES:
        ${OPENSCENEGRAPH_LIBRARIES}")
    message(STATUS "OPENSCENEGRAPH_INCLUDE_DIR:
        ${OPENSCENEGRAPH_INCLUDE_DIR}")
else (OPENSCENEGRAPH_FOUND)
    message(FATAL_ERROR "OpenSceneGraph not found")
endif (OPENSCENEGRAPH_FOUND)

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
    message(STATUS "OSGEARTHFEATURES_LIBRARY:
        ${OSGEARTHFEATURES_LIBRARY}")
    message(STATUS "OSGEARTHUTIL_LIBRARY:
        ${OSGEARTHUTIL_LIBRARY}")
    message(STATUS "OSGEARTHQT_LIBRARY:
        ${OSGEARTHQT_LIBRARY}")
    message(STATUS "OSGEARTHSYMBOLOGY_LIBRARY:
        ${OSGEARTHSYMBOLOGY_LIBRARY}")
    message(STATUS "OSGEARTHANNOTATION_LIBRARY:
        ${OSGEARTHANNOTATION_LIBRARY}")
    message(STATUS "OPENSCENEGRAPH_INCLUDE_DIR:
        ${OPENSCENEGRAPH_INCLUDE_DIR}")
else (OPENTHREADS_FOUND)
    message(FATAL_ERROR "openthreads not found")
endif (OPENTHREADS_FOUND)

# message(STATUS "Searching for GDAL")
# find_package(GDAL REQUIRED)
# if (NOT GDAL_FOUND)
#   message (FATAL_ERROR "GDAL dependency was not found!")
# endif (NOT GDAL_FOUND)

# find_package(GEOS REQUIRED)
# if (NOT GEOS_FOUND)
#   message (FATAL_ERROR "GEOS dependency was not found!")
# endif (NOT GEOS_FOUND)

# find_package(PostgreSQL REQUIRED)
# if (PostGreSQL_FOUND)
#     message(STATUS "PostgreSQL version:
#         ${PostgreSQL_VERSION_STRING}")
# else (PostGreSQL_FOUND)
#     message(STATUS "PostgreSQL not found")
# endif (PostGreSQL_FOUND)

include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OSGEARTH_INCLUDE_DIR}
        ${QGIS_INCLUDE_DIR}
        ${QT_INCLUDE_DIR}
        # ${PostgreSQL_INCLUDE_DIR}
        # ${GDAL_INCLUDE_DIR}
        # ${GEOS_INCLUDE_DIR}
)

#link_directories("${OUR_DEV_PACKAGE}/lib/")

add_executable(${PROJECT_NAME}
        main.cpp
        GraphicsWindowQt.cpp)

target_link_libraries(${PROJECT_NAME}
        ${OPENTHREADS_LIBRARY}
        ${OSGEARTH_LIBRARY}
        ${OSGEARTHFEATURES_LIBRARY}
        ${OSGEARTHUTIL_LIBRARY}
#        ${OSGEARTHQT_LIBRARY}
        ${OSGEARTHSYMBOLOGY_LIBRARY}
        ${OSGEARTHANNOTATION_LIBRARY}
        ${OPENSCENEGRAPH_LIBRARIES}
        ${QGIS_CORE_LIBRARY}
        ${QGIS_GUI_LIBRARY}
         Qt5::Core
         Qt5::Widgets
         Qt5::Xml
         Qt5::OpenGL
         Qt5::Gui
        # ${GEOS_LIBRARY}
        # ${GDAL_LIBRARY}
        # ${PostgreSQL_LIBRARIES}
        )

message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
if (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    message(STATUS "Copy assets file to ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    message(STATUS "Copy platforms file to ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    add_custom_target(
            copy_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/data"
    )
    add_custom_target(
            copy_platforms
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT5PLUGINS}/platforms "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/platforms"
    )
else()
    message(STATUS "Copy assets file to ${CMAKE_CURRENT_BINARY_DIR}")
    message(STATUS "Copy platforms file to ${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_target(
            copy_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data
    )
    add_custom_target(
            copy_platforms
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT5PLUGINS}/platforms ${CMAKE_CURRENT_BINARY_DIR}/platforms
    )
endif()

add_dependencies(${PROJECT_NAME} copy_assets copy_platforms)