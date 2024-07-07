# This file is part of ArtadoBot Crawler.
# 
# ArtadoBot Crawler is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
# 
# ArtadoBot Crawler is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
# 
# You should have received a copy of the GNU General Public License along with
# ArtadoBot Crawler. If not, see <https://www.gnu.org/licenses/>.
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

find_library(ODBC_LIBRARY NAMES odbc32 odbc)
find_path(ODBC_INCLUDE_DIR sql.h)
if (ODBC_LIBRARY AND ODBC_INCLUDE_DIR)
    message(STATUS "Found ODBC library: ${ODBC_LIBRARY}")
    message(STATUS "Found ODBC include directory: ${ODBC_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "ODBC library or include directory not found")
endif()
include_directories(${ODBC_INCLUDE_DIR})


add_executable(crawler main.cpp visit.cpp visit.hpp queue.cpp queue.hpp core.cpp core.hpp config.hpp db.cpp db.hpp)

if(MSVC)
  target_compile_options(crawler PRIVATE /W4 /WX)
else()
  target_compile_options(crawler PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-pragmas)
endif()

target_link_libraries(crawler ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES} ${ODBC_LIBRARY})