// ============================================================================
// Topic 01 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <iostream>
#include <memory>
#include <string>

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

// make_unique: an toàn exception, không cần gõ `new`.
std::unique_ptr<Sensor> make_sensor(const std::string& name) {
  return std::make_unique<Sensor>(name);
}

// make_shared: gộp 1 lần cấp phát cho object + control block.
std::shared_ptr<Sensor> make_shared_sensor(const std::string& name) {
  return std::make_shared<Sensor>(name);
}

int main() {
  auto u = make_sensor("lidar");
  CHECK(u != nullptr);
  CHECK(u && u->name == "lidar");
  CHECK(u && u->read() == 1);

  std::unique_ptr<Sensor> u2 = std::move(u);   // chuyển quyền sở hữu
  CHECK(u == nullptr);
  CHECK(u2 && u2->name == "lidar");

  auto s1 = make_shared_sensor("imu");
  CHECK(s1 != nullptr);
  CHECK(s1.use_count() == 1);

  std::shared_ptr<Sensor> s2 = s1;             // copy -> chia sẻ ownership
  CHECK(s1.use_count() == 2);
  CHECK(s2.use_count() == 2);

  {
    auto s3 = s1;
    CHECK(s1.use_count() == 3);
  }
  CHECK(s1.use_count() == 2);

  s2.reset();                                  // bỏ quyền sở hữu của s2
  CHECK(s1.use_count() == 1);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
