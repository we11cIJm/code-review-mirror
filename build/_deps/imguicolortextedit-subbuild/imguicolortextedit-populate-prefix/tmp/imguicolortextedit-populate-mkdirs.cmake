# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-src"
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-build"
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix"
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix/tmp"
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix/src/imguicolortextedit-populate-stamp"
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix/src"
  "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix/src/imguicolortextedit-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix/src/imguicolortextedit-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/artem/code-review/build/_deps/imguicolortextedit-subbuild/imguicolortextedit-populate-prefix/src/imguicolortextedit-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
