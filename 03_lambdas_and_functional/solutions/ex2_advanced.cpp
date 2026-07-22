// ============================================================================
// Topic 03 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
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

class Publisher {
public:
  std::vector<std::function<void(int)>> subs_;

  void connect(std::function<void(int)> cb) { subs_.push_back(std::move(cb)); }

  void publish(int v) {
    for (auto& cb : subs_) cb(v);
  }
};

class Subscriber {
public:
  int last_value = -1;
  int count = 0;

  void on_message(int v) {
    last_value = v;
    ++count;
  }

  void subscribe_to(Publisher& pub) {
    // Mẫu ROS 2: nối hàm thành viên vào dispatcher.
    pub.connect(std::bind(&Subscriber::on_message, this, std::placeholders::_1));
  }
};

int main() {
  using namespace std::placeholders;

  Publisher pub;
  Subscriber sub;
  sub.subscribe_to(pub);

  pub.publish(42);
  CHECK(sub.last_value == 42);
  CHECK(sub.count == 1);

  pub.publish(7);
  CHECK(sub.last_value == 7);
  CHECK(sub.count == 2);

  auto add = [](int a, int b) { return a + b; };
  std::function<int(int)> add10 = std::bind(add, 10, _1);
  CHECK(add10(5) == 15);

  auto twice = [](auto x) { return x + x; };
  CHECK(twice(3) == 6);
  CHECK(twice(2.5) == 5.0);
  CHECK(twice(std::string("ab")) == "abab");

  int base = 0;
  std::function<int()> counter = [base]() mutable { return ++base; };
  CHECK(counter() == 1);
  CHECK(counter() == 2);
  CHECK(base == 0);

  auto ptr = std::make_unique<int>(100);
  auto own = [p = std::move(ptr)] { return *p; };
  CHECK(own() == 100);
  CHECK(ptr == nullptr);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
