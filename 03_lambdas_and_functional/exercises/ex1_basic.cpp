// ============================================================================
// Topic 03 — Lambdas & Functional — Bài 1 (BASIC)
// Mục tiêu: cú pháp lambda, bắt theo value [=] vs theo reference [&],
//           lưu callback trong std::function<void(int)> và gọi lại.
//
// Cách làm: điền code vào các chỗ có "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <functional>
#include <vector>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// Một "sổ đăng ký sự kiện" nhỏ: giữ danh sách callback và phát sự kiện tới tất cả.
class EventBus {
public:
  // TODO 1: khai báo một vector các callback kiểu std::function<void(int)>.
  // std::vector<std::function<void(int)>> callbacks_;

  // TODO 2: subscribe() nhận một callback và lưu vào vector.
  void subscribe(std::function<void(int)> cb) {
    (void)cb; // TODO: push cb vào vector callbacks_
  }

  // TODO 3: publish(v) gọi lần lượt tất cả callback đã đăng ký với giá trị v.
  void publish(int v) {
    (void)v; // TODO: duyệt callbacks_ và gọi cb(v)
  }

  // Số callback đang giữ (dùng cho CHECK). Trả 0 khi chưa có vector.
  size_t size() const {
    return 0; // TODO: trả callbacks_.size()
  }
};

int main() {
  // --- 1) Lambda cơ bản trả về giá trị ---
  auto add = [](int a, int b) { return a + b; };
  CHECK(add(2, 3) == 5);

  // --- 2) Bắt theo value [=] vs theo reference [&] ---
  int x = 10;
  // TODO 4: by_val bắt x THEO GIÁ TRỊ (chụp nhanh giá trị hiện tại của x).
  std::function<int()> by_val = [] { return 0; }; // TODO: sửa để bắt x theo value
  // TODO 5: by_ref bắt x THEO THAM CHIẾU (đọc giá trị x mới nhất khi gọi).
  std::function<int()> by_ref = [] { return 0; }; // TODO: sửa để bắt x theo reference
  (void)x; // (dòng chống cảnh báo cho stub; có thể xoá sau khi bạn điền TODO 4/5)

  x = 99; // thay đổi x SAU khi tạo hai lambda

  CHECK(by_val() == 10);  // by_val nhớ giá trị lúc tạo (10)
  CHECK(by_ref() == 99);  // by_ref thấy giá trị mới nhất (99)

  // --- 3) Sổ đăng ký callback với std::function ---
  EventBus bus;
  int sum = 0;
  int last = 0;

  // TODO 6: subscribe hai callback:
  //   - callback A: cộng dồn giá trị nhận được vào `sum` (bắt sum theo reference).
  //   - callback B: ghi giá trị nhận được vào `last` (bắt last theo reference).
  // bus.subscribe(...);
  // bus.subscribe(...);

  CHECK(bus.size() == 2);

  bus.publish(5);
  CHECK(sum == 5);
  CHECK(last == 5);

  bus.publish(7);
  CHECK(sum == 12);   // 5 + 7
  CHECK(last == 7);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
