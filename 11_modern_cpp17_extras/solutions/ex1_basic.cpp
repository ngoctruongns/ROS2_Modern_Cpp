// ============================================================================
// Topic 11 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// enum class, constexpr (+ static_assert), const vs constexpr, [[nodiscard]].
// ============================================================================
#include <iostream>
#include <string>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

enum class RobotState { Idle, Moving, Error };

const char* to_string(RobotState s) {
  switch (s) {
    case RobotState::Idle:   return "Idle";
    case RobotState::Moving: return "Moving";
    case RobotState::Error:  return "Error";
  }
  return "?";
}

// constexpr: dùng được cả compile-time lẫn runtime. Vòng lặp OK từ C++14.
constexpr int factorial(int n) {
  int r = 1;
  for (int i = 2; i <= n; ++i) r *= i;
  return r;                       // factorial(0) == 1
}

// Cài đặt xong -> static_assert kiểm tra ngay lúc biên dịch.
static_assert(factorial(5) == 120, "factorial sai");
static_assert(factorial(0) == 1);

// [[nodiscard]]: cảnh báo nếu caller bỏ qua giá trị trả về.
[[nodiscard]] bool is_operational(RobotState s) {
  return s != RobotState::Error;
}

int main() {
  RobotState s = RobotState::Idle;
  CHECK(s == RobotState::Idle);
  CHECK(std::string(to_string(RobotState::Moving)) == "Moving");
  CHECK(static_cast<int>(RobotState::Error) == 2);

  // constexpr làm kích thước mảng (bắt buộc biết lúc biên dịch):
  int buf[factorial(3)] = {};     // = 6 phần tử
  (void)buf;
  CHECK(sizeof(buf) / sizeof(buf[0]) == 6);

  CHECK(factorial(4) == 24);      // cùng hàm chạy lúc chạy

  const int runtime_const = static_cast<int>(s);
  CHECK(runtime_const == 0);

  bool ok = is_operational(RobotState::Moving);
  CHECK(ok == true);
  CHECK(is_operational(RobotState::Error) == false);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
