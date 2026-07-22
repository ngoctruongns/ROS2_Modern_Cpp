// ============================================================================
// Topic 01 — Smart Pointers — Bài 1 (BASIC)
// Mục tiêu: unique_ptr, shared_ptr, make_unique/make_shared, reference count.
//
// Cách làm: điền code vào các chỗ có "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <memory>
#include <string>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

struct Sensor {
  std::string name;
  int reads = 0;
  explicit Sensor(std::string n) : name(std::move(n)) {}
  int read() { return ++reads; }
};

// TODO 1: Trả về một unique_ptr<Sensor> sở hữu một Sensor tên `name`.
//         Dùng std::make_unique. (Đừng dùng new.)
std::unique_ptr<Sensor> make_sensor(const std::string& name) {
  (void)name;
  return nullptr; // TODO: thay bằng make_unique
}

// TODO 2: Trả về một shared_ptr<Sensor> dùng std::make_shared.
std::shared_ptr<Sensor> make_shared_sensor(const std::string& name) {
  (void)name;
  return nullptr; // TODO
}

int main() {
  // --- unique_ptr ---
  auto u = make_sensor("lidar");
  CHECK(u != nullptr);
  CHECK(u && u->name == "lidar");
  CHECK(u && u->read() == 1);

  // TODO 3: chuyển quyền sở hữu từ u sang u2 (u phải thành nullptr sau đó).
  std::unique_ptr<Sensor> u2 /* = ... */;
  CHECK(u == nullptr);       // sau khi move, u rỗng
  CHECK(u2 && u2->name == "lidar");

  // --- shared_ptr & reference count ---
  auto s1 = make_shared_sensor("imu");
  CHECK(s1 != nullptr);
  CHECK(s1.use_count() == 1);

  // TODO 4: tạo s2 cùng chia sẻ quyền sở hữu với s1.
  std::shared_ptr<Sensor> s2 /* = ... */;
  CHECK(s1.use_count() == 2);
  CHECK(s2.use_count() == 2);

  {
    auto s3 = s1;            // scope tạm
    CHECK(s1.use_count() == 3);
  }
  CHECK(s1.use_count() == 2); // s3 hết scope -> count giảm

  // TODO 5: reset s2 để count về 1.
  // ...
  CHECK(s1.use_count() == 1);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
