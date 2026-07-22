// ============================================================================
// Topic 06 — std::chrono & Time — Bài 2 (ADVANCED)
// Mục tiêu: tự viết một helper kiểu rclcpp::Rate/timer.
//   - should_fire(elapsed, period): true khi elapsed >= period
//   - ticks_in_window(period, window): đếm số lần "bắn" vừa trong 1 cửa sổ
//   - period_to_hz(period): đổi period ra Hz kiểu double
// DETERMINISTIC: mọi CHECK dựa trên duration TRUYỀN VÀO / TÍNH RA,
//                KHÔNG dựa vào thời gian thực (steady_clock chỉ để minh hoạ).
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
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

// duration giây kiểu double (dùng cho tần số lẻ).
using dsec = duration<double>;

// TODO 1: Trả true khi đã trôi đủ thời gian để timer "bắn": elapsed >= period.
//         Hai đối số là nanoseconds để so sánh chung một đơn vị mịn.
bool should_fire(nanoseconds elapsed, nanoseconds period) {
  (void)elapsed; (void)period;
  return false; // TODO
}

// TODO 2: Đếm số tick trọn vẹn của `period` nằm trong `window`.
//         Vd period=100ms, window=550ms -> 5 (5*100=500 <= 550, 6*100=600 > 550).
//         Gợi ý: window / period (phép chia hai duration cho ra số nguyên).
long ticks_in_window(milliseconds period, milliseconds window) {
  (void)period; (void)window;
  return -1; // TODO
}

// TODO 3: Đổi period ra tần số Hz kiểu double.  Hz = 1 / period_giây.
//         Gợi ý: duration_cast<dsec>(period).count().
double period_to_hz(nanoseconds period) {
  (void)period;
  return -1.0; // TODO
}

int main() {
  // --- should_fire ---
  CHECK(should_fire(100ms, 100ms) == true);    // đúng bằng -> bắn
  CHECK(should_fire(150ms, 100ms) == true);    // vượt -> bắn
  CHECK(should_fire(99ms, 100ms) == false);    // chưa đủ -> chưa bắn

  // --- ticks_in_window ---
  CHECK(ticks_in_window(100ms, 550ms) == 5);
  CHECK(ticks_in_window(100ms, 1000ms) == 10);
  CHECK(ticks_in_window(250ms, 1000ms) == 4);
  CHECK(ticks_in_window(100ms, 50ms) == 0);

  // --- period_to_hz (double) ---
  CHECK(std::fabs(period_to_hz(100ms) - 10.0) < 1e-9);
  CHECK(std::fabs(period_to_hz(20ms) - 50.0) < 1e-9);
  CHECK(std::fabs(period_to_hz(400ms) - 2.5) < 1e-9);   // period lẻ -> Hz lẻ

  // --- minh hoạ đo thời gian bằng steady_clock (KHÔNG assert trên số thực) ---
  auto t0 = steady_clock::now();
  auto t1 = steady_clock::now();
  auto measured = duration_cast<nanoseconds>(t1 - t0);
  CHECK(measured.count() >= 0);   // monotonic: không bao giờ âm

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
