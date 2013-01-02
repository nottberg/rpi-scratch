cmake_minimum_required( VERSION 2.8 )
project( RPIExperiments )

include(FindPkgConfig)

#include(cmake/OptionalSource.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#find_package(Boost REQUIRED)

pkg_check_modules(GSTREAMER gstreamer-0.10)
pkg_check_modules(GSTREAMER_BASE gstreamer-base-0.10)
pkg_check_modules(GSTREAMER_APP gstreamer-app-0.10)
pkg_check_modules(GSTREAMER_CDDA gstreamer-cdda-0.10)
pkg_check_modules(GSTREAMER_TAG gstreamer-tag-0.10)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(GOBJECT gobject-2.0)
pkg_check_modules(GIO gio-2.0)

# Set up definitions and paths
link_directories(${GSTREAMER_LIBRARY_DIRS})

include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBCONFIG_INCLUDE_DIRS})

add_definitions( ${GSTREAMER_DEFINITIONS} )

add_executable( gstaudio src/gst-audio.c )
target_link_libraries( gstaudio ${GSTREAMER_LIBRARIES} )

add_executable( gstwebcam src/gst-webcam.c )
target_link_libraries( gstwebcam ${GSTREAMER_LIBRARIES} )
