// ============================================================================
// Topic 06 — std::chrono & Time — Bài 1 (BASIC)
// Mục tiêu: duration + chrono literals (500ms, 2s), duration_cast, .count(),
//           đổi chu kỳ <-> tần số. Tất cả TÍNH được, không cần sleep thật.
//
// Cách làm: điền code vào các chỗ "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <chrono>
#include <iostream>

using namespace std::chrono;
using namespace std::chrono_literals;   // bật hậu tố ms, s, ...

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// TODO 1: Trả về số MILLISECOND tương ứng với một khoảng `seconds` cho trước.
//         Dùng duration_cast<milliseconds>(...).count(). (2s -> 2000)
long seconds_to_ms(seconds s) {
  (void)s;
  return -1; // TODO
}

// TODO 2: Đổi một chu kỳ (period) ra tần số Hz (dạng số nguyên).
//         Hz = 1 / period_giây. Vd period = 100ms -> 10 Hz.
//         Gợi ý: dùng duration<double> để lấy giây, rồi 1.0 / giây.
int period_to_hz(milliseconds period) {
  (void)period;
  return -1; // TODO
}

// TODO 3: Đổi một tần số Hz ra chu kỳ tính bằng millisecond (số nguyên).
//         period_ms = 1000 / hz. Vd 50 Hz -> 20 ms.
long hz_to_period_ms(int hz) {
  (void)hz;
  return -1; // TODO
}

int main() {
  // --- literal tạo ra đúng kiểu ---
  auto p = 500ms;
  auto q = 2s;
  CHECK(p.count() == 500);        // 500ms -> 500
  CHECK(q.count() == 2);          // 2s    -> 2

  // --- duration_cast đổi đơn vị ---
  CHECK(duration_cast<milliseconds>(2s).count() == 2000);
  CHECK(duration_cast<seconds>(1500ms).count() == 1);   // cắt cụt, KHÔNG làm tròn
  CHECK(duration_cast<milliseconds>(1min).count() == 60000);

  // --- các hàm bạn phải viết ---
  CHECK(seconds_to_ms(3s) == 3000);
  CHECK(seconds_to_ms(0s) == 0);

  CHECK(period_to_hz(100ms) == 10);   // 10 Hz
  CHECK(period_to_hz(20ms) == 50);    // 50 Hz

  CHECK(hz_to_period_ms(10) == 100);  // 10 Hz -> 100ms
  CHECK(hz_to_period_ms(50) == 20);   // 50 Hz -> 20ms

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
