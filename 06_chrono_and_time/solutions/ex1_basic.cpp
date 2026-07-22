// ============================================================================
// Topic 06 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <chrono>
#include <iostream>

using namespace std::chrono;
using namespace std::chrono_literals;

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// duration_cast<milliseconds> rồi lấy .count().
long seconds_to_ms(seconds s) {
  return duration_cast<milliseconds>(s).count();
}

// Đổi period ra Hz: lấy giây kiểu double rồi 1.0 / giây.
int period_to_hz(milliseconds period) {
  double sec = duration_cast<duration<double>>(period).count();
  return static_cast<int>(1.0 / sec);
}

// Hz -> chu kỳ ms: 1000 / hz.
long hz_to_period_ms(int hz) {
  return 1000L / hz;
}

int main() {
  auto p = 500ms;
  auto q = 2s;
  CHECK(p.count() == 500);
  CHECK(q.count() == 2);

  CHECK(duration_cast<milliseconds>(2s).count() == 2000);
  CHECK(duration_cast<seconds>(1500ms).count() == 1);
  CHECK(duration_cast<milliseconds>(1min).count() == 60000);

  CHECK(seconds_to_ms(3s) == 3000);
  CHECK(seconds_to_ms(0s) == 0);

  CHECK(period_to_hz(100ms) == 10);
  CHECK(period_to_hz(20ms) == 50);

  CHECK(hz_to_period_ms(10) == 100);
  CHECK(hz_to_period_ms(50) == 20);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
