# 05 — Templates & Generic Programming (Template & lập trình tổng quát)

> **Vì sao topic này quan trọng cho ROS 2?** rclcpp là *một rừng template*.
> `node->create_publisher<std_msgs::msg::String>(...)`, `create_subscription<MsgT>(...)`,
> `node->create_client<SrvT>(...)` đều là **function template** tham số hoá theo *kiểu message*.
> Hiểu template = hiểu vì sao một API duy nhất dùng được cho *mọi* loại message.

---

## A. Basic

### 1. Function template — `template<typename T>`
Viết **một** hàm chạy cho **nhiều** kiểu. Compiler *sinh* (instantiate) một bản riêng cho mỗi kiểu.

```cpp
template<typename T>
T max_of(T a, T b) { return (a < b) ? b : a; }

max_of(3, 4);       // T = int  (suy luận / deduced)
max_of(1.5, 2.5);   // T = double
max_of<double>(1, 2.5); // ép T = double (explicit) để 2 tham số cùng kiểu
```

- **Deduced args**: compiler tự suy `T` từ đối số.
- **Explicit args**: `func<double>(...)` — chỉ định tay khi không suy được hoặc muốn ép kiểu.

### 2. Class template — `template<typename T> class`
```cpp
template<typename T>
class Stack {
  std::vector<T> data_;
public:
  void push(const T& v) { data_.push_back(v); }
  T pop() { T v = data_.back(); data_.pop_back(); return v; }
  bool empty() const { return data_.empty(); }
  std::size_t size() const { return data_.size(); }
};

Stack<int> si;        // phải chỉ định kiểu (class template không suy luận như C++17 CTAD... trừ khi có guide)
Stack<std::string> ss;
```

### 3. Alias template — `template<typename T> using`
Đặt tên gọn cho một kiểu template hoá:

```cpp
template<typename T>
using Vec = std::vector<T>;      // Vec<int>  ==  std::vector<int>

template<typename T>
using StackPtr = std::shared_ptr<Stack<T>>;
```

### 4. Non-type template parameter — `template<int N>`
Tham số template là **giá trị** (hằng compile-time), không phải kiểu:

```cpp
template<typename T, int N>
struct FixedArray { T data[N]; static constexpr int size = N; };

FixedArray<float, 3> v3;   // mảng 3 phần tử, kích thước biết lúc biên dịch
```

### 5. Vì sao template phải nằm trong header?
Compiler cần **thấy toàn bộ định nghĩa** để sinh code cho từng kiểu cụ thể tại nơi gọi.
Nếu chỉ khai báo ở `.hpp` và định nghĩa ở `.cpp` → **linker error** (undefined reference).
→ Template thường viết **trọn vẹn trong header** (đó là lý do rclcpp headers rất "nặng").

---

## B. Nâng cao / Advanced

### 6. Variadic template — `template<typename... Args>`
Nhận **số lượng đối số tuỳ ý**. `Args` là một *parameter pack*.

```cpp
// C++17 fold expression: (init op ... op pack)
template<typename... Args>
auto sum(Args... args) { return (... + args); }   // = ((a0 + a1) + a2) + ...

sum(1, 2, 3);          // 6
sum(1.0, 2.0, 3.0, 4); // 10.0
```

Trước C++17 phải đệ quy (recursion) tách `first` + `rest...`:

```cpp
template<typename T> T sum_rec(T v) { return v; }
template<typename T, typename... R> T sum_rec(T v, R... rest) { return v + sum_rec(rest...); }
```

### 7. `if constexpr` — rẽ nhánh lúc biên dịch
Nhánh **sai** bị *loại bỏ hoàn toàn* khi biên dịch (không cần biên dịch được với mọi T):

```cpp
template<typename T>
std::string describe() {
  if constexpr (std::is_integral_v<T>)        return "integral";
  else if constexpr (std::is_floating_point_v<T>) return "floating";
  else                                        return "other";
}
```

### 8. Type traits — `<type_traits>`
Truy vấn / biến đổi kiểu lúc biên dịch. Dùng biến `_v` và alias `_t` (C++17):

```cpp
std::is_integral_v<int>          // true
std::is_same_v<int, long>        // false
std::is_floating_point_v<double> // true
std::remove_reference_t<int&>    // int
```

### 9. `std::enable_if` + SFINAE (nói qua)
**SFINAE** = *Substitution Failure Is Not An Error*: nếu thay kiểu vào mà "hỏng", overload đó
bị **loại khỏi tập ứng viên** thay vì báo lỗi. `enable_if` bật/tắt overload theo điều kiện:

```cpp
template<typename T,
         typename = std::enable_if_t<std::is_integral_v<T>>>
T only_ints(T x) { return x; }   // chỉ hợp lệ khi T là kiểu nguyên
```

> **Lưu ý (KHÔNG dùng trong code C++17 ở đây):** C++20 **concepts**/`requires` thay thế phần
> lớn SFINAE, đọc dễ hơn nhiều. Repo này giữ C++17 nên ta minh hoạ bằng `if constexpr` + traits.

### 10. Template specialization — chuyên biệt hoá
Cung cấp bản cài đặt riêng cho một kiểu cụ thể:

```cpp
template<typename T> struct TypeName            { static const char* get() { return "other"; } };
template<>           struct TypeName<int>       { static const char* get() { return "int";   } };
template<>           struct TypeName<double>    { static const char* get() { return "double";} };
```

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
auto pub = node->create_publisher<std_msgs::msg::String>("chatter", 10);
//                               ^^^^^^^^^^^^^^^^^^^^^^^^ explicit template arg = kiểu message
auto sub = node->create_subscription<sensor_msgs::msg::Image>(
    "image", rclcpp::QoS(10),
    [](sensor_msgs::msg::Image::SharedPtr msg) { /* ... */ });
```

- `create_publisher<MsgT>` / `create_subscription<MsgT>` là **function template** tham số hoá
  theo *kiểu message* → một API duy nhất phục vụ *mọi* loại message.
- Bên trong, rclcpp dùng type traits + đặc điểm của `MsgT` (vd cách serialize) để chọn code phù
  hợp lúc biên dịch — chính là kiểu "dispatch theo kiểu" mà `if constexpr`/traits minh hoạ.
- Message type thường truyền **explicit** (`<MsgT>`) vì không suy luận được từ tham số.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Định nghĩa template trong `.cpp` → linker error. Để **trong header**.
- ❌ `max_of(1, 2.0)` → suy luận `T` mâu thuẫn (int vs double). Ép: `max_of<double>(1, 2.0)`.
- ❌ Dùng `if` thường thay vì `if constexpr` khi các nhánh chỉ hợp lệ với một số kiểu → lỗi biên dịch.
- ❌ Quên `typename` trước dependent type: `typename T::value_type`.
- ❌ Nghĩ template làm chậm runtime: không — nó sinh code lúc biên dịch (đổi lại: bloat + build lâu).
- ❌ Nhầm non-type param nhận giá trị runtime: `template<int N>` cần **hằng compile-time**.

## E. Docs
- https://en.cppreference.com/w/cpp/language/templates
- https://en.cppreference.com/w/cpp/language/fold
- https://en.cppreference.com/w/cpp/language/if#Constexpr_if
- https://en.cppreference.com/w/cpp/header/type_traits
- Effective Modern C++ — Items 1–4, 26–28 (type deduction, universal refs).

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — `max_of` (function template), `Stack<T>` (class template),
   alias template, non-type template param; kiểm thử với nhiều kiểu.
2. `exercises/ex2_advanced.cpp` — `sum(...)` variadic + fold expression, và `describe<T>()`
   dùng `if constexpr` + type traits (mô phỏng dispatch theo kiểu như chọn cách serialize message).

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
