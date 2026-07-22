// ============================================================================
// Topic 03 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <iostream>
#include <functional>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

class EventBus {
public:
  std::vector<std::function<void(int)>> callbacks_;

  void subscribe(std::function<void(int)> cb) {
    callbacks_.push_back(std::move(cb));
  }

  void publish(int v) {
    for (auto& cb : callbacks_) cb(v);
  }

  size_t size() const { return callbacks_.size(); }
};

int main() {
  auto add = [](int a, int b) { return a + b; };
  CHECK(add(2, 3) == 5);

  int x = 10;
  std::function<int()> by_val = [x] { return x; };   // copy giá trị x=10
  std::function<int()> by_ref = [&x] { return x; };  // tham chiếu tới x

  x = 99;

  CHECK(by_val() == 10);
  CHECK(by_ref() == 99);

  EventBus bus;
  int sum = 0;
  int last = 0;

  bus.subscribe([&sum](int v) { sum += v; });
  bus.subscribe([&last](int v) { last = v; });

  CHECK(bus.size() == 2);

  bus.publish(5);
  CHECK(sum == 5);
  CHECK(last == 5);

  bus.publish(7);
  CHECK(sum == 12);
  CHECK(last == 7);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
