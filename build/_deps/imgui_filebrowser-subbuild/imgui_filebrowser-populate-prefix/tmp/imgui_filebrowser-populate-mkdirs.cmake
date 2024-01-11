# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-src"
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-build"
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix"
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix/tmp"
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix/src/imgui_filebrowser-populate-stamp"
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix/src"
  "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix/src/imgui_filebrowser-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix/src/imgui_filebrowser-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/artem/code-review/build/_deps/imgui_filebrowser-subbuild/imgui_filebrowser-populate-prefix/src/imgui_filebrowser-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
