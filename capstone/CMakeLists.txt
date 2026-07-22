# Capstone dùng CMake riêng vì cần thư mục include/ (mini_ros framework).
cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED PROJECT_NAME)                 # build độc lập
  project(capstone LANGUAGES CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  add_compile_options(-Wall -Wextra)
endif()

find_package(Threads REQUIRED)

# Đề bài của bạn -> ./build/robot_app
add_executable(capstone_robot exercises/robot_app.cpp)
target_include_directories(capstone_robot PRIVATE include)
target_link_libraries(capstone_robot PRIVATE Threads::Threads)
set_target_properties(capstone_robot PROPERTIES OUTPUT_NAME robot_app)

# Lời giải -> ./build/sol_robot_app
add_executable(capstone_sol_robot solutions/robot_app.cpp)
target_include_directories(capstone_sol_robot PRIVATE include)
target_link_libraries(capstone_sol_robot PRIVATE Threads::Threads)
set_target_properties(capstone_sol_robot PROPERTIES OUTPUT_NAME sol_robot_app)
