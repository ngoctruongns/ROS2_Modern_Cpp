# 12 — Design Patterns & C++ Idioms (Mẫu thiết kế & thành ngữ C++ trong ROS 2)

> **Vì sao topic này?** ROS 2 *được xây bằng* các mẫu thiết kế kinh điển: pub/sub chính là
> **Observer**, `pluginlib` chính là **Factory** (+ **Strategy** cho plugin nạp vào), rclcpp
> dùng **PIMPL** để giữ ổn định ABI, còn message bus của Capstone dùng **type erasure**.
> Hiểu các mẫu này ở mức C++ giúp bạn đọc source rclcpp và thiết kế node sạch hơn.
> Topic này thuần C++ (không cần cài ROS), nhưng ví dụ mang "mùi" ROS.

---

## A. Basic

Bốn mẫu nền tảng bạn gặp *mỗi ngày* trong ROS 2. Mỗi mẫu ghi rõ nó xây trên topic C++ nào.

### 1. Observer / Publish–Subscribe — trái tim của ROS 2 (xây trên topic 03, 01)
Một **Subject** (topic) giữ danh sách **observer** và gọi lại chúng khi có sự kiện. Đây chính
là ý tưởng pub/sub: publisher không biết ai đang nghe, chỉ "phát" ra.

```cpp
struct Observer { virtual void on_event(int v) = 0; virtual ~Observer() = default; };
std::vector<std::weak_ptr<Observer>> subs;              // weak: không giữ observer sống mãi (topic 01)
void notify(int v) {
  for (auto& w : subs) if (auto s = w.lock()) s->on_event(v);   // bỏ qua observer đã chết
}
```
> Dùng `std::function` thay cho interface cũng được (topic 03) — đó là type erasure (xem B).

### 2. Strategy — hành vi cắm-rút được / pluggable behavior (topic 03, 10)
Đóng gói một **thuật toán** sau một interface, đổi thuật toán lúc chạy mà không sửa client.
Ví dụ ROS: đổi bộ điều khiển vận tốc hoặc thuật toán né vật cản khi đang chạy.

```cpp
struct Controller { virtual double compute(double err) = 0; virtual ~Controller() = default; };
struct PController : Controller { double kp; double compute(double e) override { return kp * e; } };
robot.set_controller(std::make_unique<PController>());   // cắm chiến lược khác lúc runtime
```

### 3. Factory / Factory Method — tạo object theo tên (topic 01, 05)
Tạo đối tượng dựa trên **khoá/tên chuỗi** thay vì gọi thẳng constructor. Đây chính là ý tưởng
đằng sau `pluginlib`: một `std::map<std::string, hàm-tạo>` để "đăng ký" rồi "tra cứu".

```cpp
using Creator = std::function<std::unique_ptr<Controller>()>;
std::map<std::string, Creator> registry;
registry["p"] = []{ return std::make_unique<PController>(); };
auto c = registry.at("p")();                 // tra theo tên -> object; tên lạ -> xử lý riêng
```

### 4. Singleton — CẢNH BÁO: thường bị lạm dụng (topic 09)
Đảm bảo *đúng một* thể hiện toàn tiến trình (vd một logger). C++11 bảo đảm khởi tạo
**thread-safe** cho biến `static` cục bộ ("Meyers Singleton").

```cpp
class Logger { public: static Logger& instance() { static Logger inst; return inst; } };
```
> ⚠️ **Đừng lạm dụng.** Singleton là *global state trá hình*: khó test, khó mock, ẩn phụ thuộc.
> Chấp nhận được cho logger toàn tiến trình. Với hầu hết mọi thứ khác, hãy **truyền tham số**
> (dependency injection) — chính vì vậy ROS 2 truyền `node` vào chỗ cần thay vì dùng singleton.

---

## B. Nâng cao / Advanced

### 5. PIMPL — Pointer to IMPLementation (tường lửa biên dịch) (topic 01, 10)
Giấu toàn bộ dữ liệu thành viên sau một con trỏ tới lớp `Impl` khai báo trong `.cpp`. Lợi ích:
đổi phần cài đặt **không phải biên dịch lại** client, và giữ **ổn định ABI**.

```cpp
// .hpp
class Node { public: Node(); ~Node(); void spin();
private: struct Impl; std::unique_ptr<Impl> p_; };   // header không lộ chi tiết
// .cpp: struct Node::Impl { ... }; Node::Node():p_(std::make_unique<Impl>()){} Node::~Node()=default;
```
> **rclcpp dùng PIMPL nội bộ** ở nhiều lớp để giữ ABI ổn định giữa các bản vá.
> Lưu ý: destructor phải định nghĩa trong `.cpp` (nơi `Impl` đã đầy đủ), nếu không `unique_ptr`
> không huỷ được kiểu chưa hoàn chỉnh.

### 6. CRTP — Curiously Recurring Template Pattern (đa hình tĩnh) (topic 05)
Lớp cha nhận chính lớp con làm tham số template: `class D : public Base<D>`. Cho phép "đa hình"
**không có chi phí virtual call** — gọi hàm được nối tĩnh lúc biên dịch.

```cpp
template <class Derived> struct Shape {
  double area() const { return static_cast<const Derived*>(this)->area_impl(); }
};
struct Circle : Shape<Circle> { double r; double area_impl() const { return 3.14159 * r * r; } };
```

### 7. Type erasure — giấu kiểu cụ thể sau interface đồng nhất (topic 01, 05)
`std::function` và `std::shared_ptr<void>` "xoá" kiểu cụ thể để lưu/truyền đồng nhất.

```cpp
std::function<void(int)> cb = [](int v){ /* ... */ };   // xoá kiểu của lambda/hàm
std::shared_ptr<void> any = std::make_shared<MyMsg>();   // giữ bất kỳ kiểu nào, huỷ đúng dtor
auto typed = std::static_pointer_cast<MyMsg>(any);       // lấy lại khi biết kiểu
```
> **Message bus của Capstone dùng đúng cơ chế này**: một `map` từ tên topic tới danh sách
> callback đã xoá kiểu, để mọi loại message đi qua cùng một đường ống.

### 8. RAII scope-guard — chạy dọn dẹp khi ra khỏi scope (topic 10)
Một object nhỏ chạy hàm `cleanup` trong destructor → dọn dẹp **đảm bảo** kể cả khi có exception.

```cpp
struct ScopeGuard { std::function<void()> f; ~ScopeGuard() { if (f) f(); } };
ScopeGuard g{[]{ std::cout << "đóng tài nguyên\n"; }};   // chạy khi hết scope
```

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
// Observer      : create_subscription<T>(topic, qos, cb) — publisher không biết subscriber nào
// Factory       : pluginlib::ClassLoader nạp plugin theo tên chuỗi từ file .xml
// Strategy      : plugin đã nạp (controller / planner) được gọi qua interface chung
// PIMPL         : rclcpp giấu chi tiết cài đặt để giữ ABI ổn định
// State         : rclcpp_lifecycle::LifecycleNode ~ mẫu State (Unconfigured/Inactive/Active/...)
// Strategy(sched): Executor + callback group là chiến lược lập lịch callback
```
- **pub/sub = Observer** — chuẩn xác về mặt mẫu thiết kế.
- **pluginlib = Factory** (đăng ký/tra cứu theo tên) **+ Strategy** cho plugin được nạp.
- **Lifecycle Node ≈ State pattern** (chỉ nhắc để bạn liên hệ, không đào sâu ở đây).
- **Executor / callback-group** = một chiến lược (Strategy) để lập lịch chạy callback.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ **Singleton = global state ẩn**: khó test/mock, tạo phụ thuộc vô hình. Ưu tiên truyền tham số.
- ❌ **Over-engineering**: nhồi mẫu thiết kế vào chỗ một hàm nhỏ là đủ. Mẫu là để *giảm* độ phức tạp.
- ❌ **Quên virtual destructor** trong lớp cơ sở đa hình (factory trả `Base*`/`unique_ptr<Base>`)
  → huỷ sai, rò tài nguyên. Xem topic 10.
- ❌ **Dangling observer**: Subject giữ con trỏ thô tới observer đã chết → crash. Dùng `weak_ptr`
  và tự dọn (prune) khi `expired()`. Xem topic 01.
- ❌ **CRTP nhầm kiểu**: `class D : public Base<Wrong>` — `static_cast` sẽ hỏng ngầm.

## E. Docs
- Refactoring.Guru — Design Patterns: https://refactoring.guru/design-patterns
- GoF — *Design Patterns: Elements of Reusable Object-Oriented Software* (Gamma et al.).
- cppreference — `std::function`: https://en.cppreference.com/w/cpp/utility/functional/function
- ROS 2 pluginlib: https://docs.ros.org/en/rolling/Tutorials/Beginner-Client-Libraries/Pluginlib.html
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — **Strategy + Factory**: interface `VelocityController`, hai chiến
   lược (`ConstantController`, `ProportionalController`), và `ControllerFactory` tạo theo tên
   (mô phỏng `pluginlib`: đăng ký/tra cứu trong `std::map`).
2. `exercises/ex2_advanced.cpp` — **Observer/Pub-Sub chuẩn với `weak_ptr`**: `Subject<T>` giữ
   `weak_ptr` tới observer, `notify` bỏ qua observer đã chết và **tự prune** (liên hệ topic 01).

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
