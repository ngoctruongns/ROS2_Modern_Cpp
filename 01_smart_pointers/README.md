# 01 — Smart Pointers (Con trỏ thông minh)

> **Vì sao đây là topic #1?** rclcpp *sống bằng* `shared_ptr`. `rclcpp::Node::SharedPtr`,
> publisher/subscription/timer đều là `std::shared_ptr`. Nếu bạn nắm chắc smart pointer,
> 50% cú pháp ROS 2 tự nhiên trở nên dễ hiểu.

---

## A. Basic

### 1. Tại sao smart pointer? / Why?
Con trỏ thô (`new`/`delete`) dễ gây **memory leak**, **double free**, **dangling pointer**.
Smart pointer áp dụng **RAII**: tài nguyên được giải phóng **tự động** khi ra khỏi scope.
→ Quy tắc vàng ROS 2/Modern C++: **gần như không bao giờ gõ `new`/`delete` bằng tay.**

### 2. `std::unique_ptr<T>` — sở hữu độc quyền / exclusive ownership
- Chỉ **một** owner tại một thời điểm. Không copy được, chỉ **move**.
- Không tốn chi phí (zero-overhead) so với con trỏ thô.
- Tạo bằng `std::make_unique<T>(args...)` (C++14+).

```cpp
auto p = std::make_unique<int>(42);   // p sở hữu int
auto q = std::move(p);                 // chuyển quyền sở hữu; p giờ = nullptr
// return std::move không cần thiết: return by value đã move tự động
```

### 3. `std::shared_ptr<T>` — sở hữu chia sẻ / shared ownership
- Nhiều owner cùng giữ; có bộ đếm tham chiếu (**reference count**).
- Đối tượng bị huỷ khi count về 0.
- Tạo bằng `std::make_shared<T>(args...)` — **1 lần cấp phát** cho cả object + control block
  (nhanh & an toàn hơn `shared_ptr<T>(new T)`).

```cpp
auto a = std::make_shared<std::string>("hi");
auto b = a;                 // use_count() == 2
std::cout << a.use_count(); // 2
```

### 4. `make_*` — luôn ưu tiên / always prefer
- An toàn exception, ngắn gọn, và `make_shared` gộp cấp phát.
- Ngoại lệ: cần **custom deleter** thì phải dùng constructor `shared_ptr(ptr, deleter)`.

---

## B. Nâng cao / Advanced

### 5. `std::weak_ptr<T>` — quan sát, không sở hữu / non-owning observer
- Không tăng ref count → dùng để **phá vòng lặp tham chiếu** (cyclic reference) gây leak.
- Muốn dùng phải `lock()` để lấy `shared_ptr` tạm (kiểm tra object còn sống không).

```cpp
std::weak_ptr<Foo> w = shared;
if (auto s = w.lock()) { /* object còn sống, dùng s */ }
else                   { /* đã bị huỷ */ }
```

### 6. `enable_shared_from_this<T>`
Khi một object cần tự trả về `shared_ptr` **tới chính nó** (mà không tạo control block thứ 2).

```cpp
class Node : public std::enable_shared_from_this<Node> {
  void register_self() { registry.push_back(shared_from_this()); }
};
```
> **Đây chính xác là cơ chế ROS 2 dùng**: nhiều API cần `node->shared_from_this()` (vd một
> số hàm tạo interface). Node phải được tạo bằng `make_shared`, nếu không `shared_from_this()`
> sẽ ném `bad_weak_ptr`.

### 7. Bẫy vòng lặp tham chiếu / Cyclic reference leak
`A` giữ `shared_ptr<B>` và `B` giữ `shared_ptr<A>` → count không bao giờ về 0 → **leak**.
Cách sửa: một chiều dùng `weak_ptr` (thường là chiều "con → cha").

### 8. Custom deleter
```cpp
auto fp = std::shared_ptr<FILE>(fopen("f","r"), &fclose);  // tự fclose khi hết dùng
```

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
auto node = std::make_shared<MyNode>();          // Node là shared_ptr
rclcpp::spin(node);                              // executor giữ shared_ptr tới node
auto pub  = node->create_publisher<Msg>("t",10); // pub là shared_ptr
// Trong class Node kế thừa rclcpp::Node, có thể dùng this->shared_from_this()
```
- `rclcpp::Node::SharedPtr`  ≡  `std::shared_ptr<rclcpp::Node>`
- Đừng lưu `Node*` thô lâu dài; giữ `weak_ptr` nếu cần tránh giữ node sống.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ `shared_ptr<T> p(raw); shared_ptr<T> q(raw);` → **hai control block**, double free.
- ❌ Gọi `shared_from_this()` khi object *không* được quản lý bởi `shared_ptr` → `bad_weak_ptr`.
- ❌ Vòng lặp `shared_ptr` hai chiều → leak. Dùng `weak_ptr` một chiều.
- ❌ `std::move` một `shared_ptr` rồi vẫn dùng nó (đã thành `nullptr`).

## E. Docs
- https://en.cppreference.com/w/cpp/memory/unique_ptr
- https://en.cppreference.com/w/cpp/memory/shared_ptr
- https://en.cppreference.com/w/cpp/memory/weak_ptr
- Effective Modern C++ — Items 18–22.

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — ownership của `unique_ptr`/`shared_ptr`, `make_*`, ref count.
2. `exercises/ex2_advanced.cpp` — phá vòng lặp bằng `weak_ptr` + `enable_shared_from_this`
   (mô phỏng một Node ROS 2 tự đăng ký chính nó).

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
