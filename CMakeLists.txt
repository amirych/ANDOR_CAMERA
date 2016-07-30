project(ANDOR_CAMERA)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DANDOR_CAMERA_EXPORT)
#add_definitions(-DANDOR_CAMERA_DEPRECATED_ENABLED) # enable deprecated feautures for ANDOR SDK

#
# required libraries
#

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(CFITIO cfitsio)
    if (CFITSIO_FOUND)
        message(STATUS "CFITSIO was found!")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
find_package(Qt5Core)
find_package(Qt5Network)

set(ANDOR_API_WRAPPER_LIB andor_api_wrapper)
aux_source_directory(./camera/ ANDOR_API_WRAPPER_SRC)
add_library(${ANDOR_API_WRAPPER_LIB} ${ANDOR_API_WRAPPER_SRC})
target_link_libraries(${ANDOR_API_WRAPPER_LIB} Qt5::Core)
