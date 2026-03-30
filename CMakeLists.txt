cmake_minimum_required(VERSION 3.24)
project(easyFEM)

# Workaround for CLang and RayLib's compound literals
# See: https://github.com/raysan5/raylib/issues/1343
set(CMAKE_CXX_STANDARD 17)

# ===== Dependencies =====
include(FetchContent)

# Eigen - header-only library
set(EIGEN_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/Eigen")
include_directories(${EIGEN_INCLUDE_DIR})

include(cmake/raylib.cmake)
include(cmake/imgui.cmake)
include(cmake/rlimgui.cmake)

# ===== Our Project =====
set(SOURCE_FILES
    main.cpp
    src/main_window.cpp
    src/mesh/geometry.cpp
    src/mes/mes.cpp
        src/solver/gauss.cpp
    src/mes/matrix/matrix.cpp
    src/app_ui/axis.cpp
    src/app_ui/grid.cpp
        src/saving_data.cpp
        src/saving_data.h
        src/visual/visualisation.cpp
        src/visual/visualisation.h
        src/mesh/afm.cpp
        src/mesh/afm.h
        src/solver/cholesky_ldl.cpp
        src/solver/cholesky_ldl.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Define the data directory and make it available to the code
set(DATA_DIR "${CMAKE_SOURCE_DIR}/Data")
target_compile_definitions(${PROJECT_NAME} PRIVATE DATA_DIR="${DATA_DIR}")

target_link_libraries(${PROJECT_NAME} rlimgui)

# Add imgui-src to include directories
include_directories(${CMAKE_SOURCE_DIR}/_deps/imgui-src)
include_directories(${CMAKE_SOURCE_DIR}/_deps/imgui-src)

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

# Copy the data files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Data/
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/Data/)