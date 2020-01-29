# Copyright (C) 2013, 2014, 2018 Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>

cmake_minimum_required(VERSION 3.4)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 5.6.0 COMPONENTS Gui Widgets)

# QVR is NOT required! If available it provides support for VR, e.g. HTC Vive
find_package(QVR 2.0.0 QUIET)
if (QVR_FOUND)
    link_directories(${QVR_LIBRARY_DIRS})
endif()

add_library(libcgbase STATIC
    cgopenglwidget.hpp cgopenglwidget.cpp
    cgnavigator.hpp cgnavigator.cpp
    cggeometries.hpp cggeometries.cpp
    cgtools.hpp cgtools.cpp
    tiny_obj_loader.h tiny_obj_loader.cc)
set_target_properties(libcgbase PROPERTIES OUTPUT_NAME cgbase)
if (QVR_FOUND)
    target_compile_definitions(libcgbase PUBLIC -DCG_HAVE_QVR)
    target_include_directories(libcgbase PUBLIC ${QVR_INCLUDE_DIRS})
    target_link_libraries(libcgbase ${QVR_LIBRARIES})
endif()
target_link_libraries(libcgbase Qt5::Gui Qt5::Widgets)
