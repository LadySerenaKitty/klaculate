configure_file(klakulate.h.in klaculate.h)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

find_library(fcgi fcgi REQUIRED)
find_library(fcgipp fcgi++ REQUIRED)
set(FastCGI ${fcgi} ${fcgipp})

find_package(Threads)

add_executable (klaculate main.cpp
misctools.cpp
ktok.cpp
klac.cpp
solver.cpp
util/base64.cpp
util/InputHelper.cpp
util/OutputHelper.cpp
util/TokenArray.cpp
util/TokenData.cpp
util/TokenHolder.cpp
)

set_property(TARGET klaculate PROPERTY CXX_STANDARD 17)
target_link_libraries(klaculate LINK_PUBLIC
	${FastCGI}
	${CMAKE_THREAD_LIBS_INIT}
	util
)
