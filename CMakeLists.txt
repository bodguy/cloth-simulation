cmake_minimum_required(VERSION 3.10)
project(cloth_simulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_library(COCOA_LIBS Cocoa REQUIRED)
find_library(IOKIT_LIBS IOKit REQUIRED)
find_library(CORE_LIBS CoreVideo REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLM REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} "main.cpp")

if(OPENGL_FOUND AND GLEW_FOUND AND GLFW3_FOUND)
  set(PROJECT_INCLUDE_DIR ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR})
  target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIR})

  set(GLEW_LIBS "/usr/local/lib/libGLEW.a")
  set(ADDITIONAL_LIBS ${COCOA_LIBS} ${IOKIT_LIBS} ${CORE_LIBS} ${GLEW_LIBS} ${OPENGL_gl_LIBRARY} ${GLFW3_LIBRARY})
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${ADDITIONAL_LIBS})
endif()