// ============================================================================
// Topic 11 — Modern C++17 Extras — Bài 1 (BASIC)
// Mục tiêu: enum class, constexpr (hàm + static_assert), const vs constexpr,
//           và attribute [[nodiscard]].
//
// Cách làm: điền code vào các chỗ "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <string>

// --- test harness nhỏ (giống mọi topic) ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// --- enum class: scoped, KHÔNG tự ép sang int ---
enum class RobotState { Idle, Moving, Error };

// Chuyển enum sang chuỗi để đọc log. (Trả về const char*.)
const char* to_string(RobotState s) {
  switch (s) {
    case RobotState::Idle:   return "Idle";
    case RobotState::Moving: return "Moving";
    case RobotState::Error:  return "Error";
  }
  return "?";
}

// TODO 1: viết hàm constexpr tính giai thừa n! (dùng vòng lặp, C++14+ cho phép).
//         Hàm này phải chạy được cả lúc biên dịch lẫn lúc chạy.
constexpr int factorial(int n) {
  (void)n;
  return 0; // TODO: thay bằng tính toán thật (1*2*...*n; factorial(0)==1)
}

// TODO 2: bỏ comment static_assert dưới đây SAU KHI đã cài đặt factorial().
//         (Để nguyên comment lúc chưa làm, nếu không sẽ vỡ biên dịch cả file.)
// static_assert(factorial(5) == 120, "factorial sai");

// TODO 3: đánh dấu hàm này [[nodiscard]] để không ai bỏ sót giá trị trả về.
//         (Hàm cho biết state có phải trạng thái "hoạt động được" không.)
/* [[nodiscard]] */ bool is_operational(RobotState s) {
  return s != RobotState::Error;
}

int main() {
  // --- enum class ---
  RobotState s = RobotState::Idle;
  CHECK(s == RobotState::Idle);
  CHECK(std::string(to_string(RobotState::Moving)) == "Moving");
  // enum class KHÔNG tự ép sang int -> phải static_cast khi cần số:
  CHECK(static_cast<int>(RobotState::Error) == 2);

  // --- constexpr dùng lúc biên dịch: kích thước mảng ---
  // TODO 4: sau khi cài factorial, đổi kích thước mảng thành factorial(3) (=6).
  int buf[1 /* TODO: factorial(3) */] = {};
  (void)buf;
  CHECK(sizeof(buf) / sizeof(buf[0]) == 6);

  // --- constexpr chạy lúc chạy vẫn đúng ---
  CHECK(factorial(4) == 24);

  // --- const vs constexpr ---
  const int runtime_const = static_cast<int>(s);  // cố định lúc chạy: OK
  CHECK(runtime_const == 0);

  // --- [[nodiscard]]: dùng giá trị trả về (không bỏ qua) ---
  bool ok = is_operational(RobotState::Moving);
  CHECK(ok == true);
  CHECK(is_operational(RobotState::Error) == false);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
