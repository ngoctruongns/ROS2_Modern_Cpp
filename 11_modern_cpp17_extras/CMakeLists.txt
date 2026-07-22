# Khuôn CMake dùng chung cho MỌI topic.
# - Build độc lập:  cd <topic> && cmake -B build && cmake --build build
# - Build từ root:  cmake -B build && cmake --build build  (không đụng tên target nhờ prefix t<NN>_)
cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED PROJECT_NAME)            # đang build độc lập trong thư mục topic
  project(topic LANGUAGES CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  add_compile_options(-Wall -Wextra)
endif()

find_package(Threads REQUIRED)

get_filename_component(_topic ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(SUBSTRING ${_topic} 0 2 _tag)   # ví dụ "01"

# Đề bài trong exercises/ -> binary tên gốc (vd ./build/ex1_basic)
file(GLOB _ex CONFIGURE_DEPENDS exercises/*.cpp)
foreach(src ${_ex})
  get_filename_component(n ${src} NAME_WE)
  add_executable(t${_tag}_${n} ${src})
  set_target_properties(t${_tag}_${n} PROPERTIES OUTPUT_NAME ${n})
  target_link_libraries(t${_tag}_${n} PRIVATE Threads::Threads)
endforeach()

# Lời giải trong solutions/ -> binary tiền tố sol_ (vd ./build/sol_ex1_basic)
file(GLOB _sol CONFIGURE_DEPENDS solutions/*.cpp)
foreach(src ${_sol})
  get_filename_component(n ${src} NAME_WE)
  add_executable(t${_tag}_sol_${n} ${src})
  set_target_properties(t${_tag}_sol_${n} PROPERTIES OUTPUT_NAME sol_${n})
  target_link_libraries(t${_tag}_sol_${n} PRIVATE Threads::Threads)
endforeach()
