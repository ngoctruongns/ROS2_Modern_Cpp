// ============================================================================
// Topic 03 — Lambdas & Functional — Bài 2 (ADVANCED)
// Mục tiêu: std::bind + placeholders, bind hàm thành viên (kiểu ROS 2),
//           generic lambda [](auto x){}, mutable, init-capture (bắt-bằng-move).
// Bối cảnh: mô phỏng cơ chế subscription của ROS 2 —
//   - Publisher giữ danh sách callback std::function<void(int)> (dispatcher).
//   - Subscriber đăng ký HÀM THÀNH VIÊN của mình qua std::bind(&..::on_message, this, _1).
//   - Khi Publisher::publish(v) chạy, on_message của Subscriber phải nhận được v.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// "Publisher" đơn giản: nơi các callback đăng ký và được phát tới (giống topic trong ROS 2).
class Publisher {
public:
  std::vector<std::function<void(int)>> subs_;

  void connect(std::function<void(int)> cb) { subs_.push_back(std::move(cb)); }

  void publish(int v) {
    for (auto& cb : subs_) cb(v);
  }
};

// "Subscriber" có hàm thành viên on_message dùng làm callback (mẫu ROS 2).
class Subscriber {
public:
  int last_value = -1;   // giá trị gần nhất nhận được
  int count = 0;         // số lần được gọi

  // TODO 1: viết on_message(int v): lưu v vào last_value và tăng count.
  void on_message(int v) {
    (void)v; // TODO
  }

  // Đăng ký chính mình vào một Publisher bằng std::bind hàm thành viên.
  void subscribe_to(Publisher& pub) {
    // TODO 2: dùng std::bind(&Subscriber::on_message, this, _1) để nối
    //         on_message của object này vào pub.
    (void)pub; // TODO: pub.connect( std::bind(...) );
  }
};

int main() {
  using namespace std::placeholders;

  // --- 1) Bind hàm thành viên qua Subscriber (kiểu ROS 2) ---
  Publisher pub;
  Subscriber sub;
  sub.subscribe_to(pub);

  pub.publish(42);
  CHECK(sub.last_value == 42);   // member callback đã nhận đúng giá trị
  CHECK(sub.count == 1);

  pub.publish(7);
  CHECK(sub.last_value == 7);
  CHECK(sub.count == 2);

  // --- 2) std::bind với placeholder & khoá tham số ---
  auto add = [](int a, int b) { return a + b; };
  (void)add; // (dòng chống cảnh báo cho stub; có thể xoá sau khi bạn điền TODO 3)
  // TODO 3: tạo add10 = bind add với a=10, b lấy từ tham số thứ nhất (_1).
  std::function<int(int)> add10 = [](int) { return 0; }; // TODO: thay bằng std::bind(...)
  CHECK(add10(5) == 15);

  // --- 3) Generic lambda: dùng cho nhiều kiểu ---
  // TODO 4: sửa twice thành generic lambda trả về x + x (hiện đang trả về x).
  auto twice = [](auto x) { return x; }; // TODO: đổi thân hàm thành return x + x;
  CHECK(twice(3) == 6);                       // int
  CHECK(twice(2.5) == 5.0);                    // double
  CHECK(twice(std::string("ab")) == "abab");   // string

  // --- 4) mutable lambda: sửa bản copy nội bộ ---
  int base = 0;
  // TODO 5: tạo counter bắt base THEO VALUE, mutable, mỗi lần gọi trả ++ (bản copy).
  std::function<int()> counter = [] { return 0; }; // TODO: [base]() mutable { return ++base; }
  CHECK(counter() == 1);
  CHECK(counter() == 2);
  CHECK(base == 0);   // base gốc KHÔNG đổi (chỉ sửa copy trong lambda)

  // --- 5) init-capture: bắt-bằng-move một unique_ptr vào lambda ---
  auto ptr = std::make_unique<int>(100);
  // TODO 6: tạo own = lambda bắt ptr bằng MOVE ([p = std::move(ptr)]) và trả *p.
  auto own = [] { return 0; }; // TODO: [p = std::move(ptr)] { return *p; }
  CHECK(own() == 100);
  CHECK(ptr == nullptr);   // ptr đã bị move vào lambda

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
