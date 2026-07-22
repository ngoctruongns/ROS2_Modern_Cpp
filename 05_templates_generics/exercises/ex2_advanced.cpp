// ============================================================================
// Topic 05 — Templates & Generics — Bài 2 (ADVANCED)
// Mục tiêu: variadic template + fold expression, if constexpr + type traits.
// Bối cảnh: describe<T>() mô phỏng "dispatch theo kiểu" — như rclcpp chọn cách
//           xử lý/serialize khác nhau tuỳ kiểu message lúc BIÊN DỊCH.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <string>
#include <type_traits>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// ----------------------------------------------------------------------------
// TODO 1: variadic `sum` bằng FOLD EXPRESSION (C++17).
//   Nhận số lượng đối số tuỳ ý, trả về tổng.
//   Gợi ý: template<typename... Args>  auto sum(Args... args) { return (... + args); }
// Stub trả về 0 -> các CHECK sẽ FAIL cho tới khi bạn sửa.
template<typename... Args>
auto sum(Args... args) {
  (void)sizeof...(args);
  return 0; // TODO: dùng fold expression (... + args)
}

// ----------------------------------------------------------------------------
// TODO 2: `count_args` — số lượng đối số, dùng sizeof...(pack).
template<typename... Args>
std::size_t count_args(Args... args) {
  (void)sizeof...(args);
  return 0; // TODO: trả về sizeof...(args)
}

// ----------------------------------------------------------------------------
// TODO 3: `describe<T>()` — trả về chuỗi tuỳ theo phân loại kiểu T.
//   Dùng if constexpr + type traits trong <type_traits>:
//     - kiểu nguyên (std::is_integral_v<T>)        -> "integral"
//     - kiểu thực   (std::is_floating_point_v<T>)  -> "floating"
//     - còn lại                                    -> "other"
//   LƯU Ý: phải là `if constexpr` (không phải `if`) để nhánh sai bị loại lúc biên dịch.
template<typename T>
std::string describe() {
  return "other"; // TODO: thay bằng if constexpr phân loại theo T
}

int main() {
  // --- variadic + fold ---
  CHECK(sum(1, 2, 3) == 6);
  CHECK(sum(1, 2, 3, 4, 5) == 15);
  CHECK(sum(10) == 10);
  CHECK(sum(1.0, 2.0, 3.5) == 6.5);

  CHECK(count_args(1, 2, 3) == 3);
  CHECK(count_args() == 0);
  CHECK(count_args("a", 1, 2.0, 'x') == 4);

  // --- if constexpr + type traits (dispatch theo kiểu) ---
  CHECK(describe<int>() == "integral");
  CHECK(describe<long>() == "integral");
  CHECK(describe<bool>() == "integral");     // bool cũng là integral
  CHECK(describe<double>() == "floating");
  CHECK(describe<float>() == "floating");
  CHECK(describe<std::string>() == "other");

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
