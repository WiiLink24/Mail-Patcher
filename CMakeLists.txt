cmake_minimum_required(VERSION 3.23)
project(WiiLink-Mail-Patcher CXX)

message(STATUS "Using toolchain file " ${CMAKE_TOOLCHAIN_FILE})

include_directories(.)
include_directories(${LIBOGCDIR}/include)
include_directories(${DEVKITPRO}/portlibs/wii/include)
include_directories(${DEVKITPRO}/portlibs/ppc/include)
link_directories(${DEVKITPRO}/portlibs/wii/lib)
link_directories(${DEVKITPRO}/portlibs/ppc/lib)
link_directories(${LIBOGCDIR}/lib/wii)

set(MACHDEP "-DGEKKO -mrvl -mcpu=750 -meabi -mhard-float")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -O2 ${CMAKE_CXX_FLAGS} ${MACHDEP}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -O2 ${CMAKE_CXX_FLAGS} ${MACHDEP} -fdiagnostics-color")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/apps/Mail-Patcher")

add_executable(wiilink-mail-patcher main.cpp patcher.cpp config.cpp utils.cpp)
target_link_libraries(wiilink-mail-patcher wiiuse bte fat ogc m)
