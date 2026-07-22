# 11 — Modern C++17 Extras (Những mảnh ghép C++17 còn lại)

> **Vì sao topic này?** Đây là nhóm tính năng "nhỏ mà quan trọng" của C++17 mà code
> ROS 2 dùng ở khắp nơi: `enum class` cho trạng thái/status, `constexpr` cho hằng số
> cấu hình (QoS depth, tên topic), `namespace` để tổ chức code node, và các attribute
> `[[nodiscard]]` để bắt lỗi bỏ sót giá trị trả về. Nắm nhóm này giúp bạn đọc header
> rclcpp trôi chảy và viết code an toàn hơn ngay khi biên dịch.

---

## A. Basic

### 1. `enum class` — scoped enum (an toàn hơn `enum` thường)
`enum` kiểu cũ (unscoped) rò tên ra scope ngoài và **tự động ép** sang `int` — dễ so sánh
nhầm giữa hai enum khác nhau. `enum class` (C++11) khắc phục:
- Tên phải có tiền tố: `RobotState::Idle` (không rò ra ngoài).
- **Không** tự ép sang `int` → phải `static_cast<int>(x)` khi cần.
- Có thể chỉ định kiểu nền: `enum class Code : uint8_t { Ok, Fail };`.

```cpp
enum class RobotState { Idle, Moving, Error };
RobotState s = RobotState::Idle;
// int x = s;                      // ❌ lỗi biên dịch — an toàn!
int x = static_cast<int>(s);       // ✅ khi thực sự cần số
```

### 2. `constexpr` — tính toán tại thời điểm biên dịch / compile-time
- **Biến** `constexpr`: giá trị *bắt buộc* biết lúc biên dịch → dùng làm kích thước mảng,
  tham số template, hoặc trong `static_assert`.
- **Hàm** `constexpr`: có thể chạy lúc biên dịch (nếu tham số là hằng) *hoặc* lúc chạy.

```cpp
constexpr int kQosDepth = 10;              // hằng compile-time
constexpr int square(int n) { return n*n; }
static_assert(square(4) == 16);            // chạy ngay khi biên dịch
int arr[square(3)];                        // OK: kích thước = 9
```

### 3. `const` vs `constexpr`
- `const`: "không đổi sau khi khởi tạo" — giá trị có thể xác định **lúc chạy**.
- `constexpr`: "hằng lúc biên dịch" — mạnh hơn, luôn cũng là `const`.

```cpp
int n = read_sensor();
const int a = n;         // OK: cố định lúc chạy
// constexpr int b = n;  // ❌ n không biết lúc biên dịch
constexpr int c = 42;    // OK
```

### 4. `static_assert` — kiểm tra điều kiện lúc biên dịch
Nếu điều kiện sai → **dừng biên dịch** với thông báo. Rất hợp để "khoá" giả định về
cấu hình/kích thước. C++17 cho phép bỏ chuỗi thông báo: `static_assert(cond);`.

```cpp
static_assert(sizeof(int) == 4, "expect 32-bit int");
static_assert(kQosDepth > 0);
```

### 5. Attributes: `[[nodiscard]]`, `[[maybe_unused]]`, `[[deprecated]]`
- `[[nodiscard]]`: cảnh báo nếu **bỏ qua** giá trị trả về (hợp với hàm trả error code).
- `[[maybe_unused]]`: tắt cảnh báo "unused" cho biến/tham số cố ý không dùng.
- `[[deprecated]]`: đánh dấu API cũ; dùng sẽ có cảnh báo (có thể kèm lý do).

```cpp
[[nodiscard]] bool init();          // bỏ qua kết quả -> warning
[[deprecated("dùng init() thay thế")]] void setup();
void f([[maybe_unused]] int debug_id) { /* chỉ dùng khi build debug */ }
```

---

## B. Nâng cao / Advanced

### 6. `constexpr` function nâng cao
Trong C++17, hàm `constexpr` được viết như hàm thường (có `if`, vòng lặp, biến cục bộ).
Cùng một hàm phục vụ cả compile-time (trong `static_assert`, khởi tạo `constexpr`) lẫn runtime.

```cpp
constexpr int factorial(int n) {
  int r = 1;
  for (int i = 2; i <= n; ++i) r *= i;   // vòng lặp OK trong constexpr (C++14+)
  return r;
}
static_assert(factorial(5) == 120);       // compile-time
int k = factorial(runtime_value);         // runtime cũng chạy được
```

### 7. `if constexpr` — nhánh chọn lúc biên dịch (nhắc lại)
Nhánh không được chọn sẽ **không** được biên dịch → thay cho SFINAE/tag-dispatch rườm rà.

```cpp
template <typename T>
auto to_number(T v) {
  if constexpr (std::is_same_v<T, std::string>) return std::stoi(v);
  else                                          return v;   // nhánh còn lại bị loại
}
```

### 8. `inline` variable (C++17) — hằng số cho header-only
Trước C++17, định nghĩa biến trong header và include nhiều nơi → **lỗi multiple definition**.
`inline` variable cho phép định nghĩa **một lần trong header**, dùng chung mọi TU.
Rất hợp cho hằng cấu hình dùng khắp project.

```cpp
// trong header dùng chung:
inline constexpr int kDefaultQos = 10;
inline constexpr const char* kScanTopic = "/scan";
```

### 9. `namespace` — tổ chức code
- Nhóm code theo mô-đun; tránh đụng tên.
- **Nested namespace C++17**: `namespace a::b::c { ... }` thay cho lồng nhiều tầng.
- `namespace` alias: `namespace fs = std::filesystem;`.
- **Anonymous namespace**: đặt symbol vào *internal linkage* (chỉ thấy trong file .cpp
  hiện tại) — thay cho `static` ở file scope.

```cpp
namespace robot::control {           // C++17 nested — gọn
  constexpr double kMaxSpeed = 1.5;
}
namespace ctrl = robot::control;     // alias
namespace { int g_internal = 0; }    // chỉ thấy trong file này
```

### 10. `std::filesystem` (chỉ giới thiệu ngắn)
`<filesystem>` (C++17) cung cấp `std::filesystem::path`, `exists()`, `create_directory()`...
- Cần `#include <filesystem>`.
- Trên **g++ 9.4** các thao tác `path`/`exists` cơ bản **không cần** cờ link thêm.
  (Toolchain cũ hơn — g++7/8 — đôi khi cần `-lstdc++fs`.)

```cpp
namespace fs = std::filesystem;
fs::path p = "/etc/hostname";
bool ok = fs::exists(p);             // true/false
std::string ext = fs::path("a.yaml").extension().string();  // ".yaml"
```
> Trong bài tập ta dùng filesystem ở mức **tối thiểu** (chỉ thao tác trên `path`, không
> đụng đĩa) để tránh phụ thuộc môi trường.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
namespace my_robot {

// Trạng thái/status bằng enum class — an toàn, tự tài liệu hoá.
enum class DriveState { Stopped, Driving, Faulted };

// Hằng cấu hình bằng constexpr / inline constexpr.
inline constexpr int         kDefaultQosDepth = 10;
inline constexpr const char* kCmdVelTopic     = "/cmd_vel";

// Hàm trả mã lỗi nên [[nodiscard]] để không bị bỏ sót.
[[nodiscard]] bool configure();

}  // namespace my_robot
```
- `enum class` ↔ machine-state / diagnostic status code trong node.
- `constexpr` ↔ QoS depth mặc định, tên topic/frame, ngưỡng.
- `namespace` ↔ gom code của một package/node; ROS 2 message cũng nằm trong namespace
  (`geometry_msgs::msg::Twist`).
- `[[nodiscard]]` ↔ hàm `configure()/activate()` (lifecycle) trả bool/return-code.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ So sánh/ép `enum class` sang `int` mà quên `static_cast` → lỗi biên dịch (đây là *tính năng*).
- ❌ Dùng `const` khi cần compile-time constant (ví dụ kích thước mảng, `static_assert`) — phải `constexpr`.
- ❌ Đặt `static_assert` phụ thuộc một `constexpr` chưa cài đặt → **vỡ biên dịch** cả file.
  (Trong đề bài: để `static_assert` đó ở dạng comment/TODO cho tới khi hàm được điền.)
- ❌ Định nghĩa biến thường trong header dùng nhiều nơi → *multiple definition*. Dùng `inline`.
- ❌ Bỏ qua giá trị trả về của hàm `[[nodiscard]]` (mất mã lỗi) → nhớ kiểm tra.
- ❌ Quên `#include <filesystem>` khi dùng `std::filesystem`.

## E. Docs
- https://en.cppreference.com/w/cpp/language/enum
- https://en.cppreference.com/w/cpp/language/constexpr
- https://en.cppreference.com/w/cpp/language/attributes/nodiscard
- https://en.cppreference.com/w/cpp/language/namespace
- https://en.cppreference.com/w/cpp/language/inline
- https://en.cppreference.com/w/cpp/filesystem
- Effective Modern C++ — Items 7, 10, 15.

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — `enum class RobotState`, hàm `constexpr` kiểm tra bằng
   `static_assert` + CHECK lúc chạy, và `[[nodiscard]]` được dùng đúng.
2. `exercises/ex2_advanced.cpp` — component cấu hình lúc biên dịch bằng `constexpr`,
   `inline constexpr` kiểu header, nested namespace, và một hàm chuyển trạng thái
   nhận/trả `enum class` (mô phỏng state machine của node ROS 2).

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
