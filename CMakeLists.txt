cmake_minimum_required(VERSION 3.14)
project(NTUpload)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Widgets REQUIRED)
find_package(X11 REQUIRED)

link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

add_executable(NTUpload overlay.cpp window_list.cpp)

cmake_minimum_required(VERSION 3.5)

target_link_libraries(NTUpload Qt5::Widgets)