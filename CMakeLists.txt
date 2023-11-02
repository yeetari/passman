cmake_minimum_required(VERSION 3.20)
project(passman CXX)

option(PASSMAN_BUILD_CLI "Build CLI" ON)
option(PASSMAN_BUILD_GUI "Build GUI" ON)

if(PASSMAN_BUILD_GUI)
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
endif()

add_executable(passman)
target_compile_features(passman PRIVATE cxx_std_23)
target_link_libraries(passman PRIVATE readline)
if (PASSMAN_BUILD_GUI)
    target_link_libraries(passman PRIVATE Qt5::Widgets)
endif()

add_subdirectory(src)

file(CREATE_LINK ${CMAKE_BINARY_DIR}/passman ${CMAKE_BINARY_DIR}/passman-cli SYMBOLIC)
file(CREATE_LINK ${CMAKE_BINARY_DIR}/passman ${CMAKE_BINARY_DIR}/passman-gui SYMBOLIC)

if(PASSMAN_BUILD_GUI)
    qt5_wrap_cpp(MOC_SOURCES
        src/gui/entry_model.hh
        src/gui/tree_model.hh
        src/gui/window.hh)
    target_sources(passman PRIVATE
        ${MOC_SOURCES}
        src/gui/entry_model.cc
        src/gui/tree_model.cc
        src/gui/window.cc)
endif()
