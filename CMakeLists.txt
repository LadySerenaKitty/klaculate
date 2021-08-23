cmake_minimum_required(VERSION 3.3)
project(klakulate)

execute_process(COMMAND ./ver.sh OUTPUT_VARIABLE GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_PROJECT_VERSION_TWEAK ${GIT_COMMIT})
set(klakulate_VERSION ${GIT_COMMIT})

if (EXISTS /usr/local/include)
	include_directories(/usr/local/include)
endif()

#set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_BUILD_TYPE Debug)
set(ENV{CMAKE_BUILD_TYPE} ${CMAKE_BUILD_TYPE})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)

add_subdirectory(src)
