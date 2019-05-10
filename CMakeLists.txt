cmake_minimum_required(VERSION 3.10)
project(FilamentBackground)

set(FILAMENT_DIR "/opt/filament")
set(FILAMENT_INCLUDE ${FILAMENT_DIR}/out/debug/filament/include/)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
   set(FILAMENT_LIBDIR "${FILAMENT_DIR}/out/release/filament/lib/x86_64/")
else()
   set(FILAMENT_LIBDIR "${FILAMENT_DIR}/out/debug/filament/lib/x86_64/")
endif()
set(FILAMENT_LIBS filament filaflat filamat filabridge backend utils smol-v bluevk bluegl geometry)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++17 -fno-rtti -Wall")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
   message(STATUS "Using module to find Vulkan")
   find_package(Vulkan)
endif ()

IF (NOT Vulkan_FOUND)
   find_library(Vulkan_LIBRARY NAMES vulkan REQUIRED)
ENDIF ()
MESSAGE(STATUS "Vulkan include directory: ${Vulkan_INCLUDE_DIRS}")
MESSAGE(STATUS "Vulkan library: ${Vulkan_LIBRARIES}")

set(OpenGL_GL_PREFERENCE "GLVND")
#set(OpenGL_GL_PREFERENCE "LEGACY")
FIND_PACKAGE(OpenGL REQUIRED)
MESSAGE(STATUS "OpenGL include directory: ${OPENGL_INCLUDE_DIR}")
MESSAGE(STATUS "OpenGL library: ${OPENGL_LIBRARY}")

find_package(Threads REQUIRED)
MESSAGE(STATUS "Threads Lib: " ${CMAKE_THREAD_LIBS_INIT})
MESSAGE(STATUS "Using PTHreads?: " ${CMAKE_USE_PTHREADS_INIT})

find_package(SDL2 REQUIRED)
MESSAGE(STATUS "SDL Includes ${SDL2_INCLUDE_DIR}")
MESSAGE(STATUS "SDL Libs ${SDL2_LIBRARY}")

add_executable(background src/plumbing.cc src/views.cc src/common.h)
target_compile_options(background PRIVATE -stdlib=libc++ -std=c++17 -Wall)
target_include_directories(background PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include
                           ${FILAMENT_INCLUDE} ${OPENGL_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR})
target_link_directories(background PRIVATE ${FILAMENT_LIBDIR})
target_link_libraries(background dl Threads::Threads ${Vulkan_LIBRARIES} ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${FILAMENT_LIBS} )
target_link_options(background PRIVATE -stdlib=libc++ -std=c++14)
