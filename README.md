# Modern C++ for ROS 2 — Học & Luyện tập / Learn & Practice

> Mục tiêu / Goal: Hệ thống lại toàn bộ **Modern C++ (C++17)** cần thiết để viết node ROS 2
> (Humble / Jazzy) một cách chuẩn mực và hiệu năng cao.
>
> ROS 2 Humble và Jazzy đều **chuẩn hoá theo C++17**. Vì vậy toàn bộ repo này dùng C++17.
> Một số feature C++20 (concepts, ranges, `std::format`, `std::jthread`) chỉ được **nhắc trong
> notes** (đánh dấu `C++20+`) vì Rolling đang dần hỗ trợ, nhưng **không dùng trong code** để
> bạn compile được ngay bằng `g++ 9.4`.

---

## 1. Cách dùng repo này / How to use

Mỗi topic là một thư mục `NN_ten_topic/` gồm:

```
NN_topic/
├── README.md        # Notes song ngữ: basic + nâng cao + liên hệ ROS2 + link docs
├── CMakeLists.txt    # Build tất cả bài trong topic
├── exercises/        # ĐỀ BÀI — file có sẵn TODO, bạn tự điền
└── solutions/        # LỜI GIẢI — đối chiếu sau khi làm xong
```

### Build & chạy một topic

```bash
# Cách 1: build từng topic (khuyến nghị khi mới học)
cd 01_smart_pointers
cmake -B build && cmake --build build
./build/ex1_basic          # chạy đề bài của bạn
./build/sol_ex1_basic      # chạy lời giải

# Cách 2: build TẤT CẢ topic một lần từ thư mục gốc
cmake -B build && cmake --build build
```

> Mẹo: dùng `./build.sh 01` để build nhanh 1 topic, hoặc `./build.sh all`.

Yêu cầu: `g++ >= 9` (có sẵn), `cmake >= 3.16`. **Không cần cài ROS 2** để làm các topic C++.
Chỉ riêng phần "liên hệ ROS2" trong notes là để bạn ánh xạ sang API `rclcpp` thật.

---

## 2. Roadmap học / Learning roadmap

Học theo thứ tự này — mỗi topic xây trên topic trước. Cột "ROS 2 vì sao cần" cho biết bạn sẽ
gặp kiến thức đó ở đâu khi viết node thật.

| #  | Topic | Nội dung cốt lõi | ROS 2 — vì sao cần |
|----|-------|------------------|--------------------|
| 01 | [Smart Pointers](01_smart_pointers/) | `unique_ptr`, `shared_ptr`, `weak_ptr`, `make_*`, `enable_shared_from_this` | `rclcpp::Node::SharedPtr`, `create_publisher` trả về `shared_ptr`, `shared_from_this()` |
| 02 | [Auto & Type Deduction](02_auto_and_types/) | `auto`, `decltype`, `const`/refs, structured bindings, alias | Kiểu trả về dài của `rclcpp` (`Publisher<T>::SharedPtr`) — luôn dùng `auto` |
| 03 | [Lambdas & Functional](03_lambdas_and_functional/) | lambda, capture, `std::function`, `std::bind`, `placeholders` | Callback cho subscription / timer / service |
| 04 | [Move Semantics](04_move_semantics/) | rvalue ref, `std::move`, `std::forward`, Rule of 0/3/5 | Truyền message không copy, `pub->publish(std::move(msg))` |
| 05 | [Templates & Generics](05_templates_generics/) | function/class template, variadic, `if constexpr`, type traits | `create_publisher<MsgT>()`, message type là template |
| 06 | [Chrono & Time](06_chrono_and_time/) | `duration`, `time_point`, literals `ms`/`s`, `steady_clock` | `create_wall_timer(500ms, ...)`, `rclcpp::Time/Duration` |
| 07 | [STL Containers & Algorithms](07_stl_containers_algorithms/) | `vector`, `map`, `unordered_map`, `<algorithm>`, iterators | Buffer dữ liệu, lookup theo topic name, xử lý mảng dữ liệu sensor |
| 08 | [Vocabulary Types](08_vocabulary_types/) | `std::optional`, `std::variant`, `std::any`, `std::string_view` | `rclcpp::Parameter`, giá trị cấu hình có/không, tránh copy string |
| 09 | [Concurrency](09_concurrency/) | `thread`, `mutex`, `atomic`, `condition_variable`, `future` | Executor, callback group, chia sẻ state giữa callback |
| 10 | [Class Design & RAII](10_class_design_raii/) | RAII, Rule of 5, `virtual`/`override`/`final`, PIMPL | Kế thừa `rclcpp::Node`, quản lý tài nguyên (file, socket) |
| 11 | [Modern C++17 Extras](11_modern_cpp17_extras/) | `constexpr`, `enum class`, `[[nodiscard]]`, namespaces, `<filesystem>` | Hằng số biên dịch, mã trạng thái, tổ chức code node |
| 12 | [Design Patterns & Idioms](12_design_patterns/) | Observer/Pub-Sub, Strategy, Factory, Singleton, PIMPL, CRTP, type erasure, RAII-guard | pub/sub = Observer; `pluginlib` = Factory + Strategy; rclcpp dùng PIMPL nội bộ |
| ★  | [**Capstone**](capstone/) | Ghép tất cả topic | Xây một **mini ROS 2** (Node/Pub/Sub/Timer/Executor/Param) chạy được, không cần cài ROS |

### Lộ trình gợi ý theo tuần / Suggested schedule

- **Tuần 1 (nền tảng):** 01 → 02 → 03. Đây là 80% những gì bạn gõ hằng ngày trong ROS 2.
- **Tuần 2 (hiệu năng & generic):** 04 → 05 → 06.
- **Tuần 3 (dữ liệu & an toàn kiểu):** 07 → 08.
- **Tuần 4 (đa luồng & kiến trúc):** 09 → 10 → 11 → 12.
- **Tuần 5:** Capstone — tự xây lại luồng chính của ROS 2 để hiểu bản chất.

> Ghi chú: Topic 12 chỉ gồm **design pattern ở tầng C++** (minh hoạ standalone). Các *pattern
> kiến trúc đặc thù ROS 2* (Lifecycle Node, Composition/Components, Pluginlib, Executor/
> Callback-group) cần cài ROS 2 thật + colcon nên sẽ để dành cho một repo riêng về sau.

---

## 3. Bản đồ Modern C++ ↔ rclcpp / Cheat map

Bảng "một dòng" để bạn thấy tại sao học C++ modern là học ROS 2:

```cpp
// C++ modern feature            // Xuất hiện trong rclcpp như
std::shared_ptr<T>            →   auto node = std::make_shared<MyNode>();
std::make_shared               →   rclcpp::Node::make_shared("name")
lambda + std::bind             →   create_subscription<T>(topic, qos, callback)
std::chrono literals           →   create_wall_timer(500ms, timer_cb)
template<typename MsgT>        →   create_publisher<std_msgs::msg::String>(...)
std::move                      →   publisher->publish(std::move(message))
std::optional / variant        →   node->get_parameter(...), rclcpp::ParameterValue
std::mutex / std::atomic       →   bảo vệ dữ liệu chia sẻ giữa callback trong MultiThreadedExecutor
override / virtual              →   class MyNode : public rclcpp::Node
RAII                           →   rclcpp::init/shutdown, spin — vòng đời được quản lý tự động
```

---

## 4. Tài liệu tham khảo chính / Key references

- ROS 2 docs (bản Jazzy/Humble): https://docs.ros.org/
- rclcpp API: https://docs.ros2.org/latest/api/rclcpp/
- cppreference (tra cứu chuẩn C++): https://en.cppreference.com/
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/
- Sách gợi ý: *Effective Modern C++* (Scott Meyers) — gần như trùng khớp roadmap này.

Chúc bạn học vui! Bắt đầu ở [01_smart_pointers](01_smart_pointers/).
