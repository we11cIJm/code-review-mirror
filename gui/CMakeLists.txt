include(FetchContent)

FetchContent_Declare(
    imgui_filebrowser
    URL https://github.com/AirGuanZ/imgui-filebrowser/archive/master.zip
)
FetchContent_MakeAvailable(imgui_filebrowser)
if(NOT imgui_filebrowserContent_POPULATED)
    add_library(filebrowser INTERFACE ${CMAKE_BINARY_DIR}/_deps/imgui_filebrowser-src/imfilebrowser.h)
    target_link_libraries(filebrowser INTERFACE imgui::imgui)
    set_property(TARGET filebrowser PROPERTY CXX_STANDARD 20)
    target_include_directories(filebrowser PUBLIC INTERFACE
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps>
    $<INSTALL_INTERFACE:>)
    install(TARGETS filebrowser)
endif()

FetchContent_Declare(
    ImGuiColorTextEdit
    URL https://github.com/BalazsJako/ImGuiColorTextEdit/archive/master.zip
)
FetchContent_MakeAvailable(ImGuiColorTextEdit)
if(NOT ImGuiColorTextEditContent_POPULATED)
    add_library(ColorEditor ${CMAKE_BINARY_DIR}/_deps/imguicolortextedit-src/TextEditor.cpp ${CMAKE_BINARY_DIR}/_deps/imguicolortextedit-src/TextEditor.h)
    target_link_libraries(ColorEditor imgui::imgui)
    set_property(TARGET ColorEditor PROPERTY CXX_STANDARD 20)
    target_include_directories(ColorEditor PUBLIC
      $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps>
      $<INSTALL_INTERFACE:>
    )
    install(TARGETS ColorEditor)
endif()

add_executable(app app.cpp)
add_executable(admin_app admin_app.cpp)

target_link_libraries(app PRIVATE imgui::imgui ColorEditor filebrowser ${OPENGL_LIBRARIES})
target_link_libraries(admin_app PRIVATE imgui::imgui ColorEditor filebrowser ${OPENGL_LIBRARIES} PkgConfig::LIBGIT2 gitlib libdist)
