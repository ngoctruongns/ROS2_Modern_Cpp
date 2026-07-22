# ★ Capstone — Mini ROS 2 (Temperature Monitor)

> Bài tập tổng hợp: dùng **tất cả** kiến thức từ topic 01→11 để xây một hệ thống
> ROS 2 thu nhỏ **chạy được ngay bằng C++17 thuần** (không cần cài ROS).

Bạn sẽ tự xây một hệ thống giám sát nhiệt độ robot gồm 3 node giao tiếp qua topic —
đúng luồng **Node / Publisher / Subscription / Timer / Parameter / Executor** của ROS 2,
nhưng phần "middleware" được rút gọn thành một bus trong process, và thời gian là **thời
gian ảo** do Executor điều khiển để mọi thứ **tất định** (chạy bao nhiêu lần cũng ra kết quả
như nhau — dễ kiểm tra).

---

## 1. Kiến trúc / Architecture

```
            ┌───────────────┐  Temperature   ┌────────────────┐  Alert   ┌──────────────┐
            │  SensorNode   │ ─────────────▶ │  MonitorNode   │ ───────▶ │  AlarmNode   │
            │ timer 200ms   │  topic:        │ threshold=25.0 │ topic:   │ đếm cảnh báo │
            │ publish temp  │ "temperature"  │ so sánh + log  │ "alert"  │              │
            └───────────────┘                └────────────────┘          └──────────────┘
                     ▲                                 ▲                          ▲
                     └──────────── SingleThreadedExecutor (spin 2s, bước 100ms) ─┘
```

- **SensorNode** — mỗi 200ms publish một `Temperature` (nhiệt tăng dần: 20, 21, …).
- **MonitorNode** — nghe `temperature`; nếu vượt tham số `threshold` thì publish `Alert` và log WARN.
- **AlarmNode** — nghe `alert`; đếm số cảnh báo.

## 2. Cách chạy / Build & run

```bash
cd capstone
cmake -B build && cmake --build build
./build/robot_app        # ĐỀ của bạn (fail cho tới khi làm xong TODO)
./build/sol_robot_app    # LỜI GIẢI để đối chiếu
```

## 3. Nhiệm vụ của bạn / Your task

Mở [exercises/robot_app.cpp](exercises/robot_app.cpp) và điền 9 chỗ `TODO`. `main()` và
các `CHECK` đã viết sẵn. Đọc [include/mini_ros/mini_ros.hpp](include/mini_ros/mini_ros.hpp)
như tài liệu API (nó chính là "rclcpp thu nhỏ", có chú thích topic ở từng dòng).

Kết quả kỳ vọng khi làm đúng:
```
[WARN] [monitor] temp=26.000000 > 25.000000
... (4 dòng WARN)
[PASS] sensor->published() == 10
[PASS] monitor->received() == 10
[PASS] monitor->alerts() == 4
...
TẤT CẢ PASS 🎉
```

### Vì sao ra các con số đó? / Why these numbers?
`spin_for(2s, 100ms)` = 20 bước × 100ms. Timer 200ms fire tại t = 200,400,…,2000ms → **10 lần**.
`count` chạy 0→9 nên nhiệt = 20→29. `threshold=25`, `celsius > 25` đúng với {26,27,28,29} →
**4 alert**. (Đây là lý do dùng thời gian ảo: số liệu không phụ thuộc tốc độ máy.)

## 4. Mỗi topic xuất hiện ở đâu / Where each topic shows up

| Topic | Trong capstone |
|-------|----------------|
| 01 Smart pointers | `Node`/`Publisher`/`Subscription` là `shared_ptr`; `make_shared`; type-erasure `shared_ptr<const void>` |
| 02 auto | dùng `auto` cho các kiểu `::SharedPtr` dài |
| 03 Lambdas/functional | callback timer & subscription là lambda; bus lưu `std::function` |
| 04 Move | `publish(std::move(msg))`; `std::forward` khi đăng ký callback/timer |
| 05 Templates | `create_publisher<MsgT>`, `create_subscription<MsgT>` |
| 06 Chrono | `create_wall_timer(200ms, …)`, `duration_cast`, Executor thời gian ảo |
| 07 STL | `unordered_map` bus topic→handler, `vector` timers, `queue` callback, `std::max` |
| 08 Vocabulary types | `ParamValue = variant<...>`, `get_parameter<T>()` trả `optional` |
| 09 Concurrency | `std::mutex`/`lock_guard` bảo vệ hàng đợi callback trong `Context` |
| 10 Class/RAII | kế thừa `Node`, `virtual ~Node()`, Executor giữ vòng đời node |
| 11 C++17 extras | `enum class LogLevel`, `constexpr to_cstr`, nested namespace |

## 5. Ánh xạ sang ROS 2 thật / Mapping to real rclcpp

| mini_ros | rclcpp thật |
|----------|-------------|
| `mini_ros::init()` | `rclcpp::init(argc, argv)` |
| `class X : public mini_ros::Node` | `class X : public rclcpp::Node` |
| `Node("sensor", ctx)` | `Node("sensor")` (không cần truyền context thủ công) |
| `create_publisher<T>("t", QoS(10))` | `create_publisher<T>("t", 10)` |
| `create_subscription<T>("t", QoS, cb)` | `create_subscription<T>("t", 10, cb)` |
| `create_wall_timer(200ms, cb)` | `create_wall_timer(200ms, cb)` (giống hệt) |
| `SingleThreadedExecutor::spin_for` | `rclcpp::spin(node)` / `Executor::spin()` |
| `declare_parameter/get_parameter` | `declare_parameter/get_parameter` (giống ý tưởng) |

**Khác biệt then chốt (đã đơn giản hoá):**
- ROS thật dùng **DDS** truyền qua tiến trình/mạng; ở đây chỉ là bus trong 1 process.
- ROS thật dùng **thời gian thực** (hoặc sim time); ở đây là thời gian ảo tất định.
- ROS thật có QoS phong phú (reliability, durability…); ở đây `QoS` chỉ có `depth` tượng trưng.

## 6. Thử thách mở rộng / Stretch goals
1. Thêm node `RecorderNode` ghi lại toàn bộ `Temperature` vào một `std::vector` và in trung
   bình (dùng `std::accumulate`).
2. Thêm QoS "depth": nếu số message chờ trong hàng đợi vượt `depth` thì bỏ message cũ nhất.
3. Cho `create_wall_timer` trả về một `TimerHandle` (shared_ptr) và cho phép `cancel()`.
4. Viết `MultiThreadedExecutor` thật (dùng `std::thread` + `condition_variable` từ topic 09)
   và bảo vệ state node bằng mutex.
