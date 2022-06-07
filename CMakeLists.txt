cmake_minimum_required(VERSION 3.9.2)
project(flirMJPEG)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0) #TODO check if glib is really necessary

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

include_directories(
        ${GLIB_INCLUDE_DIRS}
        "/usr/include/spinnaker/" ## TODO spinnaker installation dir
)

link_directories(
        ${GLIB_LIBRARY_DIRS}
)

#include_directories(include)
#add_subdirectory(include/XXYY)


set(my_SOURCES
        src/main.cpp
        #### TODO src/capture/SpinnakerHandler.cpp
        src/stream/MJPEGStreamer.cpp
        src/stream/MJPEGWriter.cpp
        src/capture/WebcamHandler.cpp
)


set(my_LIBRARIES
        ${OpenCV_LIBS}
        ### TODO "Spinnaker"
        )


add_executable(flirMJPEG ${my_SOURCES})
target_link_libraries(flirMJPEG ${my_LIBRARIES})
