# 10 — Class Design, RAII, Inheritance & Polymorphism (Thiết kế class, RAII, kế thừa & đa hình)

> **Vì sao quan trọng?** Mỗi node ROS 2 mà bạn viết là một **class kế thừa `rclcpp::Node`**.
> Tài nguyên (publisher, timer, file, socket) được quản lý theo **RAII** — cấp phát trong
> constructor, tự giải phóng trong destructor. Còn **đa hình** (polymorphism) cho phép cắm
> nhiều "behavior" khác nhau (vd nhiều loại controller/driver) qua một interface chung.

---

## A. Basic

### 1. RAII — Resource Acquisition Is Initialization
Nguyên tắc: **giành tài nguyên trong constructor, trả tài nguyên trong destructor**.
Object sống thì tài nguyên còn; object ra khỏi scope → destructor chạy → tài nguyên tự trả.
→ Không rò rỉ dù có `return` sớm hay exception ném ra giữa chừng.

```cpp
class File {
  std::FILE* f_;
public:
  explicit File(const char* path) : f_(std::fopen(path, "r")) {}  // acquire
  ~File() { if (f_) std::fclose(f_); }                            // release
};
// hết scope -> ~File() tự đóng file, không cần fclose thủ công
```

### 2. Constructor member-initializer list / Danh sách khởi tạo thành viên
Khởi tạo thành viên **trước** khi vào thân constructor. Bắt buộc với `const`, tham chiếu, và
base class. Nhanh hơn gán trong thân hàm (tránh khởi tạo mặc định rồi gán lại).

```cpp
class Motor {
  const int id_;          // const -> phải init ở list
  double speed_;
public:
  Motor(int id, double s) : id_(id), speed_(s) {}   // đúng thứ tự khai báo!
};
```
> Lưu ý: thành viên được khởi tạo theo **thứ tự khai báo trong class**, KHÔNG theo thứ tự
> viết trong list. Bật `-Wall -Wextra` để cảnh báo `-Wreorder`.

### 3. `explicit` constructor
Chặn chuyển đổi ngầm (implicit conversion) không mong muốn với constructor 1 tham số.

```cpp
struct Timeout { explicit Timeout(int ms); };
void wait(Timeout t);
// wait(500);            // ❌ lỗi biên dịch vì có explicit -> an toàn
wait(Timeout{500});      // ✅ rõ ràng
```

### 4. In-class member initializer / Khởi tạo mặc định ngay trong class
```cpp
class Counter {
  int value_ = 0;        // mặc định; mọi constructor không cần lặp lại
  bool active_ = true;
};
```

### 5. `= default` và `= delete`
- `= default`: yêu cầu compiler sinh phiên bản mặc định (rõ ràng ý định).
- `= delete`: cấm hẳn một hàm (vd cấm copy để làm class **non-copyable**).

```cpp
class Unique {
public:
  Unique() = default;
  Unique(const Unique&) = delete;             // cấm copy
  Unique& operator=(const Unique&) = delete;  // cấm gán copy
};
```

### 6. Rule of 0 / 3 / 5 (nhắc lại — xem thêm topic 04)
- **Rule of 0**: nếu chỉ dùng thành viên tự quản lý (smart pointer, `std::string`, `vector`),
  ĐỪNG tự viết destructor/copy/move — để compiler lo. (Ưu tiên số 1.)
- **Rule of 3**: nếu tự viết 1 trong {destructor, copy ctor, copy assign} → thường phải viết cả 3.
- **Rule of 5**: thêm move ctor + move assign khi cần hiệu năng move.

### 7. `const` member function
Hàm không sửa trạng thái object → đánh dấu `const`. Gọi được trên object `const`.

```cpp
class Buffer {
  std::vector<int> data_;
public:
  size_t size() const { return data_.size(); }   // không sửa -> const
  void push(int x) { data_.push_back(x); }        // sửa -> không const
};
```

---

## B. Nâng cao / Advanced

### 8. Inheritance / Kế thừa & gọi constructor base
```cpp
class Base {
public:
  explicit Base(std::string name) : name_(std::move(name)) {}
protected:
  std::string name_;
};
class Derived : public Base {
public:
  Derived() : Base("child") {}    // gọi constructor base ở init-list
};
```

### 9. `virtual`, `override`, `final`
- `virtual`: cho phép **dynamic dispatch** — gọi đúng hàm của lớp dẫn xuất qua con trỏ base.
- `override`: khẳng định bạn đang ghi đè hàm ảo của base (compiler báo lỗi nếu sai chữ ký).
  **Luôn dùng `override`.**
- `final`: cấm ghi đè tiếp (trên hàm) hoặc cấm kế thừa tiếp (trên class).

```cpp
struct A            { virtual int f() const { return 1; } virtual ~A() = default; };
struct B : A        { int f() const override { return 2; } };
struct C final : B  { int f() const override final { return 3; } };
```

### 10. Pure virtual / Abstract base class / Interface
`= 0` biến hàm thành **thuần ảo** → class trở thành **trừu tượng** (không tạo instance được),
đóng vai trò **interface**. Lớp dẫn xuất phải cài đặt hết mới tạo được object.

```cpp
struct SensorDriver {
  virtual int read() = 0;              // pure virtual -> interface
  virtual ~SensorDriver() = default;  // virtual destructor bắt buộc (xem dưới)
};
```

### 11. Virtual destructor — VÌ SAO bắt buộc với base đa hình
Khi `delete base_ptr;` mà con trỏ base trỏ tới object dẫn xuất:
- Nếu destructor base **không** `virtual` → chỉ `~Base()` chạy, phần dẫn xuất **không được
  huỷ** → **undefined behavior / rò rỉ tài nguyên**.
- Nếu `virtual` → `~Derived()` chạy trước rồi tới `~Base()` → đúng và an toàn.

```cpp
struct Base    { virtual ~Base() = default; };   // ✅ có virtual dtor
struct Derived : Base { std::vector<int> big; };
std::unique_ptr<Base> p = std::make_unique<Derived>();
// p huỷ -> ~Derived() chạy đúng nhờ virtual dtor (nếu thiếu -> leak `big`)
```

### 12. Lưu trữ & gọi đa hình
```cpp
std::vector<std::unique_ptr<SensorDriver>> drivers;
drivers.push_back(std::make_unique<Lidar>());
drivers.push_back(std::make_unique<Imu>());
for (auto& d : drivers) d->read();   // gọi đúng read() của từng loại
```

### 13. PIMPL idiom (Pointer to IMPLementation) — tóm tắt
Giấu chi tiết cài đặt sau một `unique_ptr<Impl>` khai báo trước (forward-declared) trong header.
→ Giảm phụ thuộc biên dịch (build nhanh), ổn định ABI.

```cpp
// widget.hpp
class Widget {
  struct Impl;                     // chỉ khai báo
  std::unique_ptr<Impl> pimpl_;
public:
  Widget();
  ~Widget();                       // định nghĩa trong .cpp (nơi Impl đầy đủ)
};
```

### 14. Composition vs Inheritance / Kết hợp và kế thừa
- **Kế thừa** = quan hệ **"is-a"** (Lidar *là một* SensorDriver).
- **Composition** = quan hệ **"has-a"** (Robot *có một* Motor). Thường **ưu tiên composition**;
  chỉ kế thừa khi thật sự cần quan hệ is-a và đa hình.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
class MyNode : public rclcpp::Node {          // kế thừa Node (is-a Node)
public:
  MyNode() : Node("my_node") {                // gọi constructor base với tên node
    pub_ = create_publisher<Msg>("topic", 10);   // tài nguyên quản lý RAII
    timer_ = create_wall_timer(500ms, [this]{ tick(); });
  }
private:
  rclcpp::Publisher<Msg>::SharedPtr pub_;     // has-a publisher (composition)
  rclcpp::TimerBase::SharedPtr timer_;
};
```
- Bạn **kế thừa** `rclcpp::Node` và gọi `: Node("tên_node")` ở init-list.
- `rclcpp::Node` có **virtual destructor** → an toàn khi giữ node qua `Node::SharedPtr` (base).
- **Đa hình**: định nghĩa interface `Controller { virtual Cmd compute() = 0; }` rồi cắm
  `PidController`, `MpcController`… hoán đổi runtime mà không sửa node.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Quên `virtual` cho destructor của base đa hình → `delete` qua base leak/UB.
- ❌ Thứ tự init-list khác thứ tự khai báo thành viên → cảnh báo `-Wreorder`, bug tinh vi.
- ❌ Quên `explicit` → chuyển đổi ngầm ngoài ý muốn.
- ❌ Ghi đè mà quên `override` → lỡ tạo hàm mới (sai chữ ký) thay vì override → không dispatch.
- ❌ Object slicing: gán object dẫn xuất vào **biến base theo giá trị** → mất phần dẫn xuất.
  Luôn dùng con trỏ/tham chiếu (hoặc `unique_ptr<Base>`) cho đa hình.
- ❌ Tự viết destructor rỗng "cho chắc" khiến mất move mặc định (vi phạm Rule of 0).

## E. Docs
- https://en.cppreference.com/w/cpp/language/raii
- https://en.cppreference.com/w/cpp/language/constructor
- https://en.cppreference.com/w/cpp/language/virtual
- https://en.cppreference.com/w/cpp/language/abstract_class
- https://en.cppreference.com/w/cpp/language/destructor
- C++ Core Guidelines — mục C (Classes), R (Resource management).
- Effective Modern C++ — Items 11–17.

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — class `ScopedResource` theo RAII: đếm số resource đang mở bằng
   biến static, tăng ở ctor / giảm ở dtor, làm **non-copyable** (delete copy). Kiểm tra
   count đổi đúng khi vào/ra scope.
2. `exercises/ex2_advanced.cpp` — interface trừu tượng `SensorDriver` (pure virtual `read()` +
   virtual destructor); hai lớp dẫn xuất `Lidar`, `Imu`; lưu vào
   `std::vector<std::unique_ptr<SensorDriver>>` và gọi đa hình qua con trỏ base.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
