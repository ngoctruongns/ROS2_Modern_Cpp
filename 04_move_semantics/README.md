# 04 — Move Semantics (Ngữ nghĩa di chuyển)

> **Vì sao quan trọng cho ROS 2?** Message trong ROS 2 có thể rất lớn (ảnh, point cloud).
> `publisher->publish(std::move(msg))` giúp **tránh copy** cả buffer. Hiểu move semantics =
> viết code nhanh hơn, ít cấp phát bộ nhớ hơn, và đọc được API `unique_ptr` message của rclcpp.

---

## A. Basic

### 1. lvalue vs rvalue
- **lvalue**: có tên, có địa chỉ, tồn tại sau biểu thức (`int x; x;`, `arr[i]`, `*p`).
- **rvalue**: giá trị tạm, sắp biến mất (`42`, `x + 1`, `foo()` trả về theo giá trị).

```cpp
int x = 10;      // x là lvalue
int y = x + 1;   // (x + 1) là rvalue (tạm)
```

### 2. Rvalue reference `T&&`
Tham chiếu chỉ **bind vào rvalue** — nghĩa là "đối tượng này sắp bỏ đi, tôi được phép cướp ruột nó".

```cpp
void f(int& lref);    // chỉ nhận lvalue
void f(int&& rref);   // chỉ nhận rvalue (tạm)
int a = 1;
f(a);   // gọi bản lvalue
f(2);   // gọi bản rvalue
```
> Lưu ý: bản thân biến `rref` (kiểu `int&&`) là một **lvalue** vì nó có tên!

### 3. `std::move` chỉ là một phép ép kiểu / just a cast
`std::move(x)` **không** di chuyển gì cả. Nó chỉ **ép** `x` thành rvalue (`T&&`) để lần gọi
hàm tiếp theo chọn được overload move. Việc "di chuyển" thật sự do move ctor/move assignment làm.

```cpp
std::string a = "hello";
std::string b = std::move(a);  // std::move chỉ ép kiểu; move ctor mới thực sự cướp buffer
// a giờ ở trạng thái "moved-from": hợp lệ nhưng KHÔNG xác định nội dung
```

### 4. Move constructor & move assignment
```cpp
struct Buf {
  int* data; size_t n;
  Buf(Buf&& o) noexcept : data(o.data), n(o.n) {  // cướp con trỏ...
    o.data = nullptr; o.n = 0;                     // ...rồi vô hiệu hoá nguồn
  }
  Buf& operator=(Buf&& o) noexcept {
    if (this != &o) {           // chống self-move
      delete[] data;            // dọn tài nguyên hiện tại
      data = o.data; n = o.n;
      o.data = nullptr; o.n = 0;
    }
    return *this;
  }
};
```

### 5. Khi nào compiler tự sinh? / When are they generated?
- Move ctor/assignment được **tự sinh** chỉ khi bạn KHÔNG tự khai báo:
  destructor, copy ctor, copy assignment, hay move op còn lại.
- Khai báo bất kỳ hàm nào trong nhóm đó có thể **chặn** việc tự sinh move → class âm thầm chỉ còn copy (chậm). Đây là lý do có Rule of 5.

### 6. Trạng thái moved-from / moved-from state
Sau khi bị move, đối tượng nguồn phải **hợp lệ nhưng không xác định**: chỉ được phép gán lại
hoặc huỷ, không nên đọc giá trị. Với `std::string`/`std::vector` chuẩn thường là rỗng.

---

## B. Nâng cao / Advanced

### 7. Forwarding (universal) reference & perfect forwarding
Trong **template**, `T&&` KHÔNG phải rvalue reference — nó là **forwarding reference** nhờ
"reference collapsing": bind được cả lvalue lẫn rvalue.

```cpp
template <class T, class... Args>
std::unique_ptr<T> make(Args&&... args) {          // Args&& = forwarding ref
  return std::make_unique<T>(std::forward<Args>(args)...);  // giữ nguyên l/r-value
}
```
- `std::forward<Args>(a)` = ép **có điều kiện**: nếu tham số gốc là rvalue thì thành rvalue, còn lvalue thì giữ lvalue. Nhờ đó copy/move được chọn đúng.
- Dùng `std::move` cho rvalue reference thường, `std::forward` cho forwarding reference.

### 8. Rule of 0 / 3 / 5
- **Rule of 0** (ưu tiên nhất): đừng tự viết ctor/dtor gì cả; để `std::vector`, `std::string`,
  `std::unique_ptr`… tự quản lý. Class của bạn tự có copy/move đúng, miễn phí.
- **Rule of 3** (C++98): nếu cần 1 trong {destructor, copy ctor, copy assignment} thì thường cần cả 3.
- **Rule of 5** (C++11): nếu cần bất kỳ trong 5 hàm đặc biệt {dtor, copy ctor, copy=, move ctor, move=} thì cân nhắc khai báo cả 5.

### 9. `= default` / `= delete`
```cpp
struct S {
  S(S&&) = default;             // nhờ compiler sinh bản mặc định
  S(const S&) = delete;         // cấm copy (vd tài nguyên độc quyền)
};
```

### 10. `noexcept` trên move — vì sao `std::vector` cần?
Khi `std::vector` mở rộng (grow/reallocate), nó chỉ **move** phần tử cũ sang vùng nhớ mới
nếu move ctor là `noexcept`. Nếu không, để giữ **strong exception guarantee** nó sẽ **copy**
(chậm!). → Luôn đánh dấu move ctor/assignment `noexcept` khi có thể.

### 11. Copy elision / RVO / (guaranteed elision C++17)
Compiler được phép (và trong C++17 là **bắt buộc** với prvalue) bỏ qua hoàn toàn copy/move khi
trả về đối tượng tạm.

```cpp
Buf make() { return Buf(100); }  // C++17: KHÔNG copy, KHÔNG move — dựng thẳng tại chỗ
Buf b = make();                  // 0 lần move
```
> Đừng viết `return std::move(local);` — nó **cản** RVO và có thể làm chậm hơn.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
auto msg = std::make_unique<std_msgs::msg::String>();
msg->data = "hello";
publisher->publish(std::move(msg));   // move: không copy buffer của message
```
- `publish(std::move(msg))` chuyển quyền sở hữu message cho middleware → tránh copy.
- Truyền message lớn nên nhận **bằng giá trị + move vào chỗ lưu**, thay vì copy nhiều lần.
- rclcpp có API nhận `std::unique_ptr<Msg>`: cho phép **intra-process zero-copy** (trong cùng
  process, message không bị serialize/copy mà chuyển thẳng con trỏ). Đây là move semantics ở
  quy mô hệ thống.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Tưởng `std::move` tự di chuyển. Không — nó chỉ ép kiểu; phải có move ctor mới có tác dụng.
- ❌ Dùng lại đối tượng sau khi `std::move` và mong nó còn giá trị cũ.
- ❌ Quên `noexcept` trên move ops → `std::vector` âm thầm copy khi grow.
- ❌ `return std::move(local);` làm mất RVO.
- ❌ Khai báo destructor nhưng quên rằng nó **chặn** move tự sinh → class thành copy-only (chậm).
- ❌ Quên chống self-move-assignment (`if (this != &other)`) → giải phóng nhầm tài nguyên của chính mình.
- ❌ Dùng `std::move` thay vì `std::forward` trong template forwarding → move nhầm cả lvalue của người gọi.

## E. Docs
- https://en.cppreference.com/w/cpp/language/move_constructor
- https://en.cppreference.com/w/cpp/utility/move
- https://en.cppreference.com/w/cpp/utility/forward
- https://en.cppreference.com/w/cpp/language/copy_elision
- https://en.cppreference.com/w/cpp/language/rule_of_three
- Effective Modern C++ — Items 23–30.

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — lớp `Buffer` đếm copy vs move; tự viết move ctor + move assignment
   (cướp con trỏ + null hoá nguồn). Kiểm tra move KHÔNG tăng bộ đếm copy và để nguồn rỗng.
2. `exercises/ex2_advanced.cpp` — factory perfect-forwarding `make<T>(Args&&...)` + lớp Rule-of-5;
   kiểm tra số copy/move cho lvalue vs rvalue và self-move-assignment an toàn.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
