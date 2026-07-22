# 02 — auto & Type Deduction (auto & suy luận kiểu)

> **Vì sao topic này quan trọng cho ROS 2?** Kiểu trả về trong rclcpp *rất dài*
> (vd `rclcpp::Publisher<std_msgs::msg::String>::SharedPtr`). Nếu không dùng `auto`,
> code sẽ ngập trong tên kiểu. Nắm chắc quy tắc suy luận của `auto` giúp bạn viết
> code ROS 2 ngắn, đúng, và tránh những cú copy/tham chiếu ngoài ý muốn.

---

## A. Basic

### 1. `auto` suy luận kiểu như thế nào? / How does `auto` deduce?
Quy tắc cốt lõi: `auto` **loại bỏ `const` và tham chiếu ở mức trên cùng** (top-level),
giống hệt suy luận tham số template theo trị.

```cpp
const int  ci = 42;
auto x = ci;        // x là `int` (const bị bỏ) -> x là bản COPY, sửa được
int& r = /*...*/;
auto y = r;         // y là `int` (ref bị bỏ) -> COPY, không phải tham chiếu
```
→ Muốn giữ tham chiếu/const thì phải **nói rõ**: `auto&`, `const auto&`, `auto*`.

### 2. `auto&` / `const auto&` / `auto*`
```cpp
std::vector<int> v{1,2,3};
auto        a = v[0];   // int   — copy
auto&       b = v[0];   // int&  — sửa b thì v[0] đổi theo
const auto& c = v[0];   // const int& — chỉ đọc, không copy (rẻ với object lớn)
auto*       p = &v[0];  // int*  — con trỏ
```
- Object **lớn/không copy được** → ưu tiên `const auto&` để tránh copy tốn kém.
- Muốn **sửa** phần tử qua tham chiếu → `auto&`.

### 3. Khi nào `auto` giúp dễ đọc? / When does `auto` help readability?
- Kiểu dài dòng: iterator, kiểu trả về template, `shared_ptr<...>` của ROS 2.
- Tránh lặp lại tên kiểu ở vế phải (`auto p = std::make_shared<Foo>();`).
- **Không lạm dụng**: nếu kiểu cụ thể làm code rõ nghĩa hơn (vd `int count = 0;`),
  cứ ghi kiểu ra.

### 4. `decltype` — hỏi kiểu của một biểu thức / query the type
`decltype` cho ra **chính xác** kiểu khai báo của biểu thức, **giữ** cả `const`/ref.
```cpp
int i = 0;
decltype(i)   a;   // int
decltype((i)) b = i; // int&  — chú ý: (i) là lvalue biểu thức -> tham chiếu!
```

### 5. Type alias bằng `using` (và vì sao `typedef` là legacy)
```cpp
using Meters = double;                       // rõ hơn typedef
using StringVec = std::vector<std::string>;  // đọc trái->phải, dễ hơn typedef
// using còn làm được alias template (typedef thì không):
template <class T> using Vec = std::vector<T>;
```
> `typedef double Meters;` vẫn chạy nhưng `using` đọc thuận, hỗ trợ template alias
> → Modern C++ khuyến nghị dùng `using`.

### 6. Hàm trả về `auto` / `auto` return type (C++14)
```cpp
auto add(int a, int b) { return a + b; }   // suy ra int từ biểu thức return
```
- Tất cả các `return` phải cho ra **cùng một kiểu**.
- Dùng khi kiểu trả về dài hoặc phụ thuộc biểu thức; ngược lại nên ghi kiểu rõ.

---

## B. Nâng cao / Advanced

### 7. Structured bindings (C++17) — tách nhiều biến một lúc
Gỡ `pair`/`tuple`/`struct`/phần tử `map` ra nhiều tên trong một dòng.
```cpp
std::map<std::string,int> m{{"a",1}};
for (const auto& [key, val] : m)      // key, val thay cho .first/.second
  std::cout << key << "=" << val;

std::tuple<int,std::string> t{1,"x"};
auto [id, name] = t;                  // id=int, name=std::string
```
- Dùng `const auto&`/`auto&` để tránh copy hoặc để sửa tại chỗ.

### 8. `decltype(auto)` — suy luận giữ nguyên ref/const
`auto` sẽ bỏ ref/const; `decltype(auto)` **giữ đúng** kiểu của biểu thức return.
```cpp
std::vector<int> v{10};
decltype(auto) f() { return v[0]; }  // trả về int& (giữ tham chiếu)
// auto f()          -> trả về int (copy)
```
Hữu ích khi viết hàm "chuyển tiếp" (forwarding) giá trị trả về chính xác.

### 9. `auto` trong range-for
```cpp
for (auto x        : v) {}  // copy mỗi phần tử (thường không muốn)
for (auto& x       : v) {}  // tham chiếu — sửa được
for (const auto& x : v) {}  // MẶC ĐỊNH nên dùng: không copy, chỉ đọc
```

### 10. Trailing return type — `auto f() -> T`
```cpp
auto make() -> std::vector<int> { return {1,2,3}; }
// hữu ích khi kiểu trả về phụ thuộc tham số (decltype):
template <class A, class B>
auto sum(A a, B b) -> decltype(a + b) { return a + b; }
```

### 11. Forwarding reference `auto&&` (ghi chú ngắn)
`auto&&` bám theo giá trị (lvalue → `T&`, rvalue → `T&&`), dùng khi cần "bắt bất kỳ".
Trong range-for, `for (auto&& x : range)` là cách an toàn nhất cho mọi loại range
(kể cả proxy như `std::vector<bool>`).

> **C++20 (chỉ để biết, KHÔNG dùng trong bài):** C++20 cho phép `auto` làm **tham số hàm**
> (abbreviated function template) và bổ sung **concepts** để ràng buộc kiểu. Repo này
> giữ nguyên C++17 nên ta không dùng các tính năng đó.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
// Kiểu đầy đủ dài kinh khủng:
rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub =
    node->create_publisher<std_msgs::msg::String>("topic", 10);

// Với auto — sạch sẽ, vẫn đúng kiểu:
auto pub = node->create_publisher<std_msgs::msg::String>("topic", 10);
auto sub = node->create_subscription<std_msgs::msg::String>("topic", 10, cb);
auto timer = node->create_wall_timer(500ms, cb);
```
- `auto` gần như là mặc định cho mọi `create_*` của rclcpp.
- Structured bindings rất hợp khi duyệt map tham số hoặc `std::map`:
```cpp
for (const auto& [name, value] : parameter_map)
  RCLCPP_INFO(logger, "%s", name.c_str());
```

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ `auto x = big_vector;` khi chỉ muốn đọc → copy tốn kém. Dùng `const auto&`.
- ❌ `for (auto x : v)` khi muốn sửa `v` → chỉ sửa bản copy. Dùng `auto&`.
- ❌ Tưởng `auto` giữ `const`/ref — không! Top-level `const`/ref bị **bỏ**.
- ❌ Nhầm `decltype(i)` với `decltype((i))`: dấu ngoặc thêm → tham chiếu.
- ❌ Structured binding rồi mong nó là tham chiếu tới nguồn mà lại viết `auto [..]`
  trên một bản copy — dùng `auto&`/`const auto&` khi cần bám nguồn.

## E. Docs
- https://en.cppreference.com/w/cpp/language/auto
- https://en.cppreference.com/w/cpp/language/decltype
- https://en.cppreference.com/w/cpp/language/structured_binding
- https://en.cppreference.com/w/cpp/language/type_alias
- Effective Modern C++ — Items 1–6 (type deduction, `auto`), Item 9 (`using` vs `typedef`).

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — quy tắc suy luận `auto` (copy vs tham chiếu), chứng minh
   `auto&` sửa được nguồn còn `auto` (copy) thì không; type alias bằng `using`; `decltype`.
2. `exercises/ex2_advanced.cpp` — structured bindings trên `std::map<std::string,int>`,
   trên struct/tuple; `decltype(auto)`; range-for bằng `const auto&`.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
