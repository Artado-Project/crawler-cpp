cmake_minimum_required(VERSION 3.10)

project(artadobot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(artadobot main.cpp visit.cpp visit.hpp core.hpp)

target_link_libraries(artadobot ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})