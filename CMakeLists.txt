# CMake for macOS build ONLY
# Use Visual Studio .sln file on Windows.

cmake_minimum_required(VERSION 4.1.1)
project(LabraScope VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test suite" OFF)
option(USE_STATIC_LIBS "Link libraries statically" ON)
set(CMAKE_BUILD_TYPE "Release") # defines NDEBUG

# System packages
find_package(OpenGL REQUIRED)

# Imported targets
# Function to create imported static library targets
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/libs)
function(add_static_library target_name lib_path include_path)
    add_library(${target_name} STATIC IMPORTED)
    set_target_properties(${target_name} PROPERTIES
        IMPORTED_LOCATION ${EXTERNAL_DIR}/${lib_path}
    )
    if(include_path)
        target_include_directories(${target_name} INTERFACE
            ${EXTERNAL_DIR}/${include_path}
        )
    endif()
endfunction()

add_static_library(libusb "libusb/libusb-1.0.a" "libusb/")
add_static_library(fftw "fftw/libfftw3.a" "fftw/")
add_static_library(nfd "nfd/libnfd.a" "nfd/")
add_static_library(glfw "glfw/libglfw3.a" "glfw/include")
# Define Sources
set(IMGUI_DIR libs/imgui)
set(LIBRADOR_DIR libs/librador)

set(SOURCES
    src/main.cpp 
    src/util.cpp
    backends/imgui_impl_glfw.cpp 
    backends/imgui_impl_opengl3.cpp
    misc/cpp/imgui_stdlib.cpp
    ${LIBRADOR_DIR}/librador.cpp
    ${LIBRADOR_DIR}/usbcallhandler.cpp
    ${LIBRADOR_DIR}/o1buffer.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/implot.cpp
    ${IMGUI_DIR}/implot_demo.cpp
    ${IMGUI_DIR}/implot_items.cpp

)
# Add include directories
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    misc/cpp/
    backends/
    ${EXTERNAL_DIR}/exprtk
    ${IMGUI_DIR}/
    ${LIBRADOR_DIR}/
    # ${EXTERNAL_DIR}/glfw/include/
    src/
    ${EXTERNAL_DIR}/
)

target_link_libraries(${PROJECT_NAME}
    libusb
    fftw
    nfd
    glfw
)

if(APPLE)
    find_library(OPENGL_LIBRARY OpenGl)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(SECURITY_LIBRARY Security)
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARY}
        ${COCOA_LIBRARY} 
        ${IOKIT_LIBRARY} 
        ${COREVIDEO_LIBRARY}
        ${APPKIT_LIBRARY}
        ${SECURITY_LIBRARY}
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG)
endif()