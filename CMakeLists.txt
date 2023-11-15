cmake_minimum_required(VERSION 3.18)   

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

set(CMAKE_VERBOSE_MAKEFILE True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin.dbg)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin.rel)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin.relwithdbg)

project(code-review-app)

add_executable(app app.cpp)

find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/imgui-filebrowser)
include_directories(${CMAKE_SOURCE_DIR}/ImGuiColorTextEdit)

add_library(ColorEditor ${CMAKE_SOURCE_DIR}/ImGuiColorTextEdit/TextEditor.cpp ${CMAKE_SOURCE_DIR}/ImGuiColorTextEdit/TextEditor.h)
target_link_libraries(ColorEditor imgui::imgui)
set_property(TARGET ColorEditor PROPERTY CXX_STANDARD 20)
target_include_directories(ColorEditor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
install(TARGETS ColorEditor)

target_link_libraries(app PRIVATE imgui::imgui OpenGL::GL ColorEditor)
