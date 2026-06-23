cmake_minimum_required(VERSION 3.16)

project(RTWeekend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_VIEWER "Build the real-time raylib viewer" ON)

include_directories(src)

if(APPLE AND NOT DEFINED OpenMP_CXX_FLAGS)
    find_program(BREW_EXECUTABLE brew)
    if(BREW_EXECUTABLE)
        execute_process(
            COMMAND ${BREW_EXECUTABLE} --prefix libomp
            OUTPUT_VARIABLE LIBOMP_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
    foreach(candidate "${LIBOMP_PREFIX}" /opt/homebrew/opt/libomp /usr/local/opt/libomp)
        if(candidate AND EXISTS "${candidate}/lib/libomp.dylib")
            set(OpenMP_CXX_FLAGS "-Xclang -fopenmp -I${candidate}/include")
            set(OpenMP_CXX_LIB_NAMES "omp")
            set(OpenMP_omp_LIBRARY "${candidate}/lib/libomp.dylib")
            break()
        endif()
    endforeach()
endif()

find_package(OpenMP)

include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        set(CMAKE_CUDA_ARCHITECTURES 70 75 80 86)
    endif()
    set(HAVE_CUDA ON)
    message(STATUS "CUDA found: building GPU renderer")
else()
    message(STATUS "CUDA not found: building CPU-only")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
                        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
                        $<$<COMPILE_LANGUAGE:CXX>:-Wunused-variable>)
endif()

function(link_openmp target_name)
    if(NOT OpenMP_CXX_FOUND)
        return()
    endif()
    if(TARGET OpenMP::OpenMP_CXX)
        target_link_libraries(${target_name} PUBLIC OpenMP::OpenMP_CXX)
    else()
        target_compile_options(${target_name} PRIVATE ${OpenMP_CXX_FLAGS})
        target_link_libraries(${target_name} PUBLIC ${OpenMP_CXX_LIBRARIES})
    endif()
endfunction()

set(COMMON_SOURCES
    src/scene.cpp
)

add_executable(inOneWeekend
    src/main.cpp
    src/image_io.cpp
    ${COMMON_SOURCES}
)
link_openmp(inOneWeekend)

if(HAVE_CUDA)
    add_library(cuda_rt STATIC src/cuda/render.cu)
    set_target_properties(cuda_rt PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    target_include_directories(cuda_rt PUBLIC src)
    target_compile_definitions(cuda_rt PUBLIC USE_CUDA)
    target_compile_options(cuda_rt PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)

    add_executable(cuda_renderer
        src/main.cpp
        src/image_io.cpp
        ${COMMON_SOURCES}
    )
    target_link_libraries(cuda_renderer PRIVATE cuda_rt)
    link_openmp(cuda_renderer)
endif()

if(BUILD_VIEWER)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raylib)

    add_executable(viewer
        src/viewer.cpp
        ${COMMON_SOURCES}
    )
    target_link_libraries(viewer PRIVATE raylib)
    link_openmp(viewer)
    if(HAVE_CUDA)
        target_link_libraries(viewer PRIVATE cuda_rt)
    endif()
endif()
