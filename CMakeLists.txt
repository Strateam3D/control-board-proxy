cmake_minimum_required( VERSION 3.25 )

message("------------------------------")
message("Building ControlBoard")
message("------------------------------")
message("Source Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message("Binary Dir: ${CMAKE_CURRENT_BINARY_DIR}")

# -----------------------------------------------------------------------------
# CMake Configuration
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE_INIT "Debug")

project(control_board)
set(PROJECT_NAME control_board)

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------
add_subdirectory(3rdParty)
add_subdirectory(src)