add_library(libdist INTERFACE include/distributer/distributer.hpp)

target_include_directories(libdist PUBLIC INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(TARGETS libdist)

set_target_properties(libdist PROPERTIES LINKER_LANGUAGE CXX)