# 06 — std::chrono & Time (Thời gian)

> **Vì sao quan trọng với ROS 2?** Mỗi khi bạn viết `create_wall_timer(500ms, cb)` hay
> `rclcpp::Rate(10)`, bạn đang dùng `std::chrono`. Chu kỳ timer, tần số (Hz), đo thời gian
> chạy của một callback... tất cả đều xoay quanh `duration`, `time_point` và `clock`.
> Nắm chắc chrono giúp bạn không nhầm đơn vị (ms vs s) và không bị bug đo thời gian.

---

## A. Basic

### 1. `std::chrono::duration` — một khoảng thời gian / a length of time
Là một khoảng thời gian, KHÔNG phải một mốc. Có sẵn các alias:
`std::chrono::nanoseconds`, `microseconds`, `milliseconds`, `seconds`, `minutes`, `hours`.

```cpp
using namespace std::chrono;
milliseconds a{500};   // 500 ms
seconds      b{2};     // 2 s
```

### 2. Chrono literals — cú pháp ngắn / literal suffixes
```cpp
using namespace std::chrono_literals;   // bật hậu tố ms, s, min, h, us, ns
auto p = 500ms;   // std::chrono::milliseconds
auto q = 2s;      // std::chrono::seconds
```
> Đây chính là thứ bạn truyền vào `create_wall_timer(500ms, cb)`.

### 3. `.count()` — lấy số nguyên bên trong / the raw tick number
`duration` bọc một con số + một **đơn vị (period)**. `.count()` trả về con số đó
**theo đơn vị của chính nó**:
```cpp
seconds{2}.count();        // == 2   (đơn vị: giây)
milliseconds{500}.count(); // == 500 (đơn vị: mili giây)
```

### 4. `std::chrono::duration_cast` — đổi đơn vị / convert units
Đổi giữa các đơn vị KHÁC nhau phải dùng `duration_cast` (cắt cụt về số nguyên,
truncation toward zero):
```cpp
auto ms = duration_cast<milliseconds>(2s);   // 2000ms
ms.count();                                  // == 2000
duration_cast<seconds>(1500ms).count();      // == 1  (cắt cụt, không làm tròn)
```
> Chuyển từ đơn vị lớn -> nhỏ (s -> ms) là an toàn và **implicit** được;
> nhỏ -> lớn (ms -> s) mất mát nên phải `duration_cast` tường minh.

### 5. Ratio / period — nền tảng type-safe (khái niệm)
Mỗi `duration<Rep, Period>` mang một `std::ratio` mô tả "1 tick = bao nhiêu giây".
`seconds` = `ratio<1,1>`, `milliseconds` = `ratio<1,1000>`. Nhờ period nằm trong
**kiểu (type)**, compiler bắt lỗi trộn đơn vị và tự nhân/chia hệ số khi cast.

---

## B. Nâng cao / Advanced

### 6. `steady_clock` vs `system_clock` — chọn đúng đồng hồ
| Clock | Đặc điểm | Dùng cho |
|-------|----------|----------|
| `steady_clock` | **Monotonic** — không bao giờ nhảy lùi, không bị NTP/chỉnh giờ | **Đo khoảng thời gian** (interval, timeout, benchmark) |
| `system_clock` | Giờ tường (wall clock), có thể bị chỉnh lùi/nhảy | Lấy ngày giờ thực, timestamp hiển thị |

> **Quy tắc vàng:** đo "đã trôi qua bao lâu" thì LUÔN dùng `steady_clock`.
> Dùng `system_clock` để đo interval là bug kinh điển (giờ bị NTP chỉnh -> âm).

### 7. `time_point` & đo thời gian trôi qua / elapsed
`clock::now()` trả về một `time_point`. Hiệu hai `time_point` là một `duration`:
```cpp
auto t0 = std::chrono::steady_clock::now();
// ... làm việc ...
auto t1 = std::chrono::steady_clock::now();
auto elapsed = t1 - t0;                                   // duration
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
```

### 8. Custom duration & đổi ra double giây / to floating seconds
`.count()` của đơn vị nguyên bị cắt cụt. Muốn giá trị thực (vd 1.5s) dùng
`duration<double>`:
```cpp
using dsec = std::chrono::duration<double>;              // giây kiểu double
double sec = std::chrono::duration_cast<dsec>(1500ms).count();  // 1.5
```

### 9. ROS 2 có hệ thời gian RIÊNG / ROS 2 has its own time types
- `rclcpp::Time`, `rclcpp::Duration`, `rclcpp::Clock` — có thể chạy theo **sim time**
  (`/clock` khi `use_sim_time=true`) chứ không chỉ giờ hệ thống.
- **Nhưng** chu kỳ timer thì vẫn nhận `std::chrono::duration` thuần.
- Nguyên tắc: dùng `std::chrono` cho **period/timeout**; dùng `rclcpp::Clock/Time`
  cho **timestamp trong message** và logic phụ thuộc sim time.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
using namespace std::chrono_literals;

// Timer: đối số đầu là std::chrono::duration
auto timer = node->create_wall_timer(500ms, [](){ /* chạy mỗi 0.5s */ });

// Rate: giữ vòng lặp ở tần số cố định
rclcpp::WallRate rate(10);        // 10 Hz  == chu kỳ 100ms
rclcpp::Rate     rate2(2s);       // cũng nhận chrono duration
while (rclcpp::ok()) { /* ... */ rate.sleep(); }
```
- **Chu kỳ -> tần số:** `Hz = 1 / period_giây`. Vd `100ms` -> `10 Hz`.
- **Tần số -> chu kỳ:** `period_giây = 1 / Hz`. Vd `50 Hz` -> `20ms`.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Dùng `system_clock` để đo interval -> âm khi giờ bị chỉnh. Dùng `steady_clock`.
- ❌ Quên `duration_cast` khi đổi nhỏ->lớn (ms->s) -> lỗi biên dịch (mất mát ngầm).
- ❌ Tưởng `duration_cast<seconds>(1500ms)` == 2. Nó **cắt cụt** = 1, không làm tròn.
- ❌ Đọc `.count()` mà không để ý đơn vị của duration -> nhầm ms với s.
- ❌ Dùng `int` thay vì `duration<double>` khi cần giây lẻ (vd 1.5s -> mất phần lẻ).
- ❌ Truyền số trần `500` vào `create_wall_timer` -> phải là `500ms` (có đơn vị).

## E. Docs
- https://en.cppreference.com/w/cpp/chrono/duration
- https://en.cppreference.com/w/cpp/chrono/duration/duration_cast
- https://en.cppreference.com/w/cpp/chrono/steady_clock
- https://en.cppreference.com/w/cpp/chrono/system_clock
- https://docs.ros.org/en/humble/ (rclcpp Time/Duration/Clock, Rate)

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — tạo duration bằng literal, `duration_cast` đổi đơn vị,
   `.count()`, đổi chu kỳ <-> tần số (deterministic, không cần sleep thật).
2. `exercises/ex2_advanced.cpp` — tự viết một helper kiểu `Rate`/timer:
   `should_fire(elapsed, period)`, đếm số tick trong một cửa sổ thời gian,
   đổi period ra Hz (double). Tất cả CHECK trên giá trị truyền vào/tính ra,
   KHÔNG phụ thuộc thời gian thực.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
