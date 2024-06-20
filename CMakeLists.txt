cmake_minimum_required(VERSION 3.10)

project(crawler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Comment the following line out for production
set(CMAKE_BUILD_TYPE Debug)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(crawler main.cpp visit.cpp visit.hpp queue.cpp queue.hpp core.cpp core.hpp config.hpp)

if(MSVC)
  target_compile_options(crawler PRIVATE /W4 /WX)
else()
  target_compile_options(crawler PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-pragmas)
endif()

target_link_libraries(crawler ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})