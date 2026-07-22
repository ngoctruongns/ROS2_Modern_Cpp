cmake_minimum_required(VERSION 3.16)
project(ros2_modern_cpp LANGUAGES CXX)

# Toàn repo dùng C++17 — đúng chuẩn ROS 2 Humble/Jazzy.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

# Thêm từng topic. Mỗi topic tự khai báo target của nó.
add_subdirectory(01_smart_pointers)
add_subdirectory(02_auto_and_types)
add_subdirectory(03_lambdas_and_functional)
add_subdirectory(04_move_semantics)
add_subdirectory(05_templates_generics)
add_subdirectory(06_chrono_and_time)
add_subdirectory(07_stl_containers_algorithms)
add_subdirectory(08_vocabulary_types)
add_subdirectory(09_concurrency)
add_subdirectory(10_class_design_raii)
add_subdirectory(11_modern_cpp17_extras)
add_subdirectory(12_design_patterns)
add_subdirectory(capstone)
