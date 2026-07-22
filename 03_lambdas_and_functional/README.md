# 03 — Lambdas & Functional (Lambda, std::function, std::bind, callback)

> **Vì sao topic này quan trọng?** ROS 2 là kiến trúc **hướng callback**: subscription,
> timer, service, action... đều nhận một *callable* để gọi khi có sự kiện. Nắm chắc
> lambda + `std::function` + `std::bind` là nắm chắc "xương sống" cách bạn viết logic
> trong một Node.

---

## A. Basic

### 1. Cú pháp lambda / Lambda syntax
```cpp
[capture](params) -> Ret { body }
```
- `capture` — biến bên ngoài lambda được "bắt" vào.
- `params`  — tham số như hàm thường.
- `-> Ret`  — kiểu trả về (thường suy luận tự động, có thể bỏ).

```cpp
auto add = [](int a, int b) { return a + b; };  // Ret suy luận = int
int s = add(2, 3);                              // 5
```

### 2. Bắt theo giá trị `[=]` vs theo tham chiếu `[&]`
- `[=]` — **copy** biến vào lambda tại thời điểm *tạo* lambda (chụp nhanh giá trị).
- `[&]` — giữ **tham chiếu** tới biến gốc (thấy thay đổi mới nhất; cẩn thận dangling).

```cpp
int x = 10;
auto by_val = [=] { return x; };   // nhớ x = 10 mãi mãi
auto by_ref = [&] { return x; };   // đọc x hiện tại
x = 99;
by_val();  // 10
by_ref();  // 99
```

### 3. Bắt tường minh / Explicit captures & `this`
```cpp
int a = 1, b = 2;
auto f = [a, &b] { return a + b; };  // a theo value, b theo reference
```
Trong một class, bắt `this` để dùng thành viên:
```cpp
struct Counter {
  int n = 0;
  auto make() { return [this] { return ++n; }; }  // sửa n của object
};
```
> ⚠️ `[this]` bắt **con trỏ** tới object → object phải còn sống khi lambda chạy.

### 4. Lưu lambda trong `std::function<Ret(Args)>`
Mỗi lambda có kiểu **ẩn danh, duy nhất**. Muốn cất vào biến/vector/thành viên có kiểu cố
định, dùng `std::function`:
```cpp
std::function<int(int)> g = [](int x) { return x * 2; };
std::vector<std::function<void(int)>> callbacks;   // kho callback
callbacks.push_back([](int v){ std::cout << v; });
```
> `std::function` linh hoạt nhưng có chi phí (type-erasure/heap). Với callback ROS 2 thì
> hoàn toàn chấp nhận được.

---

## B. Nâng cao / Advanced

### 5. `std::bind` + `std::placeholders`
`std::bind` tạo một callable mới bằng cách "khoá sẵn" một số tham số:
```cpp
using namespace std::placeholders;   // _1, _2, ...
auto add = [](int a, int b){ return a + b; };
auto add10 = std::bind(add, 10, _1); // a=10, b lấy từ tham số thứ nhất khi gọi
add10(5);                            // 15
```

### 6. Bind một hàm thành viên / Bind a member function
Đây là **mẫu kinh điển trong tutorial ROS 2**:
```cpp
struct S {
  void on_msg(int v) { /* ... */ }
};
S s;
auto cb = std::bind(&S::on_msg, &s, _1);  // this = &s, _1 = tham số truyền vào
cb(42);                                    // gọi s.on_msg(42)
```

### 7. Generic lambda `[](auto x){}` (C++14)
Tham số kiểu `auto` → lambda hoạt động với **nhiều kiểu** (như template):
```cpp
auto twice = [](auto x) { return x + x; };
twice(3);      // int -> 6
twice(2.5);    // double -> 5.0
twice(std::string("ab"));  // "abab"
```

### 8. `mutable` lambda
Mặc định biến bắt theo `[=]` là **const** bên trong lambda. `mutable` cho phép sửa **bản
copy nội bộ** (không ảnh hưởng biến gốc):
```cpp
int count = 0;
auto tick = [count]() mutable { return ++count; }; // sửa copy riêng
tick(); // 1
tick(); // 2  (count gốc vẫn = 0)
```

### 9. Init-capture / bắt-bằng-move `[p = std::move(ptr)]` (C++14)
Cho phép tạo biến mới ngay trong capture — cách duy nhất để **move** một `unique_ptr` vào
lambda:
```cpp
auto ptr = std::make_unique<int>(7);
auto own = [p = std::move(ptr)] { return *p; };  // lambda sở hữu ptr
```

### 10. Lambda hay `std::bind`? / Tradeoff
- **Modern C++ khuyên dùng lambda** thay `std::bind`: rõ ràng hơn, dễ inline, ít lỗi
  placeholder, thông báo lỗi biên dịch dễ đọc hơn.
- Nhưng **tutorial ROS 2 dùng cả hai** → bạn cần đọc hiểu được `std::bind`.

```cpp
// Tương đương nhau:
std::bind(&S::on_msg, this, _1);
[this](int v){ on_msg(v); };
```

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
// Subscription — kiểu std::bind (rất phổ biến trong tutorial):
sub_ = create_subscription<std_msgs::msg::String>(
    "topic", 10, std::bind(&MyNode::on_msg, this, std::placeholders::_1));

// Subscription — kiểu lambda (hiện đại, gọn):
sub_ = create_subscription<std_msgs::msg::String>(
    "topic", 10,
    [this](std_msgs::msg::String::SharedPtr m) { on_msg(m); });

// Timer — hầu như luôn dùng lambda bắt this:
timer_ = create_wall_timer(500ms, [this] { on_timer(); });
```
- Callback nhận `Msg::SharedPtr` (một `shared_ptr` tới message) → xem lại topic 01.
- `this` bắt vào callback ⇒ Node phải sống lâu hơn subscription/timer (thường đúng vì Node
  sở hữu chúng).

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ `[&]` bắt tham chiếu tới biến cục bộ rồi lambda sống lâu hơn biến → **dangling**.
- ❌ Quên `_1` (hoặc thiếu `using namespace std::placeholders`) khi `std::bind` hàm thành viên.
- ❌ Bind `&S::method` mà quên truyền `this`/con trỏ object làm tham số đầu.
- ❌ Tưởng `[=]` cho sửa biến bắt — không, phải thêm `mutable` (và chỉ sửa copy).
- ❌ Cố `std::function` một lambda bắt-move `unique_ptr` — `std::function` cần callable
  *copyable*, sẽ lỗi biên dịch. Giữ nó bằng `auto` hoặc gói khác.
- ❌ `[this]` trong lambda nhưng object đã bị huỷ khi callback chạy → undefined behavior.

## E. Docs
- https://en.cppreference.com/w/cpp/language/lambda
- https://en.cppreference.com/w/cpp/utility/functional/function
- https://en.cppreference.com/w/cpp/utility/functional/bind
- https://docs.ros.org/en/humble/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Cpp-Publisher-And-Subscriber.html
- Effective Modern C++ — Items 31–34.

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — sổ đăng ký callback `std::function<void(int)>`, so sánh
   hành vi bắt-theo-value vs bắt-theo-reference.
2. `exercises/ex2_advanced.cpp` — mô phỏng subscription ROS 2: đăng ký hàm thành viên qua
   `std::bind(&Subscriber::on_message, this, _1)` vào một dispatcher kiểu Publisher; cộng
   thêm một generic lambda dùng với nhiều kiểu.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
