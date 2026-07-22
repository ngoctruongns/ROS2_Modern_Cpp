# 08 — Vocabulary Types (Kiểu "từ vựng" C++17)

> **Vocabulary types** là những kiểu chuẩn hoá cách ta *nói chuyện* trong code:
> `std::optional` ("có thể có/không có giá trị"), `std::variant` ("một trong nhiều kiểu"),
> `std::any` ("một giá trị bất kỳ, xoá kiểu"), `std::string_view` ("cửa sổ nhìn vào chuỗi,
> không sao chép"). ROS 2 dùng đúng những khái niệm này: `rclcpp::ParameterValue` chính là
> một `variant`; API tra tham số trả về kiểu mang tinh thần `optional`.

---

## A. Basic

### 1. `std::optional<T>` — "có thể có một giá trị" / maybe a value
Thay cho các quy ước cũ như trả `-1`, con trỏ `nullptr`, hoặc cặp `(bool ok, T value)`.

```cpp
#include <optional>

std::optional<int> find_port(const std::string& key);   // có thể không tìm thấy

auto p = find_port("baud");
if (p.has_value())  use(*p);        // has_value() / *  để lấy giá trị
if (p)              use(p.value()); // ép sang bool / .value() (ném nếu rỗng)
int v = p.value_or(9600);           // giá trị mặc định nếu rỗng
```
- `std::nullopt` là giá trị "rỗng" tường minh: `return std::nullopt;`
- `*opt` và `opt->field` truy cập nhanh **không kiểm tra** (giống con trỏ) — chỉ dùng khi
  đã chắc có giá trị.
- `.value()` sẽ **ném** `std::bad_optional_access` nếu rỗng → an toàn hơn `*`.

### 2. `std::string_view` — cửa sổ không sở hữu / non-owning cheap view
`string_view` = con trỏ + độ dài. **Không cấp phát, không sao chép** dữ liệu chuỗi.
Lý tưởng cho tham số hàm "chỉ đọc" nhận được cả `std::string` lẫn `const char*` literal
mà không tạo bản sao.

```cpp
#include <string_view>
size_t count_dots(std::string_view s) {           // nhận mọi loại chuỗi, 0 copy
  return std::count(s.begin(), s.end(), '.');
}
count_dots("a.b.c");                 // literal -> không tạo std::string
std::string name = "/scan";
count_dots(name);                    // std::string -> cũng không copy
auto tail = std::string_view("/turtle1/cmd_vel").substr(1); // substr trả view khác, vẫn 0 copy
```

> ⚠️ **BẪY DANGLING**: `string_view` **không sở hữu** ký tự. Đừng trả về view trỏ tới một
> `std::string` cục bộ/tạm thời — chuỗi đó chết khi hết hàm, view thành **treo (dangling)**.
> ```cpp
> std::string_view bad() { std::string s = "hi"; return s; } // ❌ view treo!
> ```

---

## B. Nâng cao / Advanced

### 3. `std::variant<Ts...>` — một trong nhiều kiểu / a type-safe union
Giữ **đúng một** giá trị trong tập kiểu cho trước; biết kiểu hiện tại là gì (an toàn kiểu,
khác hẳn `union` C thô).

```cpp
#include <variant>
std::variant<int, double, std::string> v = 42;
v.index();                       // 0 (đang là int)
std::holds_alternative<int>(v);  // true
int  i = std::get<int>(v);       // ném std::bad_variant_access nếu sai kiểu
if (auto* p = std::get_if<int>(&v)) use(*p);   // get_if: an toàn, trả con trỏ hoặc nullptr
v = std::string("hello");        // đổi sang kiểu khác
```

### 4. `std::visit` + overloaded visitor
`std::visit` gọi đúng nhánh xử lý theo kiểu đang giữ. Mẫu "overload" cổ điển (chạy tốt C++17):

```cpp
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;   // deduction guide

std::string s = std::visit(overload{
    [](int i)                { return "int:"    + std::to_string(i); },
    [](double d)             { return "double:" + std::to_string(d); },
    [](const std::string& x) { return "str:"    + x; },
}, v);
```
Hoặc đơn giản dùng chuỗi `if (auto* p = std::get_if<...>(&v))` — cả hai đều warning-clean.

### 5. `std::monostate` — trạng thái "rỗng" mặc định
`variant` không có constructor mặc định nếu kiểu đầu tiên không default-constructible.
Đặt `std::monostate` ở đầu để variant có trạng thái "chưa set".

```cpp
std::variant<std::monostate, int, std::string> v;  // mặc định = monostate (rỗng)
```

### 6. `std::any` + `std::any_cast` — xoá kiểu / type-erased single value
Chứa **một** giá trị của **bất kỳ** kiểu nào; lấy ra phải biết đúng kiểu.

```cpp
#include <any>
std::any a = 42;
int i = std::any_cast<int>(a);           // ném std::bad_any_cast nếu sai kiểu
if (auto* p = std::any_cast<int>(&a)) use(*p);   // dạng con trỏ: an toàn
a.type() == typeid(int);                 // truy vấn kiểu lúc chạy
```
> **Khi nào KHÔNG dùng `any`?** Khi tập kiểu đã biết trước và hữu hạn → dùng `variant`
> (an toàn kiểu tại compile-time, không cấp phát heap, hỗ trợ `visit`). `any` chỉ hợp khi
> tập kiểu thực sự mở/không biết trước, và bạn chấp nhận kiểm tra kiểu lúc chạy + có thể cấp
> phát heap.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

- **`rclcpp::ParameterValue` ≈ variant** trên `bool / int64_t / double / string` + các mảng
  của chúng. `rclcpp::Parameter` bọc quanh nó với một tên.
  ```cpp
  rclcpp::Parameter p("baud", 9600);
  p.get_type();                 // PARAMETER_INTEGER (giống variant.index())
  int64_t v = p.as_int();       // giống std::get<int64_t> — ném nếu sai kiểu
  ```
- **optional cho "tham số chưa set"**:
  ```cpp
  int64_t baud = 9600;
  node->get_parameter_or("baud", baud, int64_t{9600});   // lấy hoặc dùng mặc định
  // hoặc: if (node->get_parameter("baud", baud)) { ... }  // trả bool "có/không"
  ```
  Đây chính là tinh thần `optional::value_or`.
- **string_view cho tên topic/param**: xử lý, so khớp tiền tố tên topic (`"/turtle1/..."`)
  mà không sao chép chuỗi — hợp với hàm chỉ đọc, hot path.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Gọi `*opt` / `opt.value()` khi optional **rỗng** (`*` = UB, `.value()` = ném).
- ❌ Trả về `std::string_view` trỏ tới `std::string` cục bộ/tạm → **dangling view**.
- ❌ Lưu `string_view` vào struct sống lâu hơn chuỗi gốc → treo. Cần sở hữu thì dùng `std::string`.
- ❌ `std::get<T>(v)` khi variant đang giữ kiểu khác → `std::bad_variant_access`. Dùng
  `get_if` nếu không chắc.
- ❌ Dùng `std::any` trong khi `variant` là đủ → mất an toàn kiểu, thêm cấp phát heap.
- ❌ Quên `std::monostate` khiến variant không default-construct được.

## E. Docs
- https://en.cppreference.com/w/cpp/utility/optional
- https://en.cppreference.com/w/cpp/utility/variant
- https://en.cppreference.com/w/cpp/utility/variant/visit
- https://en.cppreference.com/w/cpp/utility/any
- https://en.cppreference.com/w/cpp/string/basic_string_view
- ROS 2: https://docs.ros.org/en/humble/Concepts/Basic/About-Parameters.html

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — tra cứu cấu hình trả `std::optional<int>` (tìm thấy / không),
   dùng `value_or` làm mặc định; dùng `std::string_view` để soi key mà không sao chép.
2. `exercises/ex2_advanced.cpp` — dựng `ParamValue = std::variant<bool,int64_t,double,std::string>`
   (giống `rclcpp::ParameterValue`), lưu vào `std::map`, dùng `std::visit` để "stringify"
   và truy vấn kiểu bằng `holds_alternative`.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
