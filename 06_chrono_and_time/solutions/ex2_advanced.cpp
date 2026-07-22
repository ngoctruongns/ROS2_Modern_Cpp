// ============================================================================
// Topic 06 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <chrono>
#include <cmath>
#include <iostream>

using namespace std::chrono;
using namespace std::chrono_literals;

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

using dsec = duration<double>;

// So sánh duration: chúng tự quy về đơn vị chung, an toàn kiểu.
bool should_fire(nanoseconds elapsed, nanoseconds period) {
  return elapsed >= period;
}

// window / period cho ra số tick nguyên (phép chia hai duration -> Rep).
long ticks_in_window(milliseconds period, milliseconds window) {
  if (period.count() <= 0) return 0;
  return window / period;
}

// Hz = 1 / period_giây; dùng duration<double> để giữ phần lẻ.
double period_to_hz(nanoseconds period) {
  double sec = duration_cast<dsec>(period).count();
  return 1.0 / sec;
}

int main() {
  CHECK(should_fire(100ms, 100ms) == true);
  CHECK(should_fire(150ms, 100ms) == true);
  CHECK(should_fire(99ms, 100ms) == false);

  CHECK(ticks_in_window(100ms, 550ms) == 5);
  CHECK(ticks_in_window(100ms, 1000ms) == 10);
  CHECK(ticks_in_window(250ms, 1000ms) == 4);
  CHECK(ticks_in_window(100ms, 50ms) == 0);

  CHECK(std::fabs(period_to_hz(100ms) - 10.0) < 1e-9);
  CHECK(std::fabs(period_to_hz(20ms) - 50.0) < 1e-9);
  CHECK(std::fabs(period_to_hz(400ms) - 2.5) < 1e-9);

  auto t0 = steady_clock::now();
  auto t1 = steady_clock::now();
  auto measured = duration_cast<nanoseconds>(t1 - t0);
  CHECK(measured.count() >= 0);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
