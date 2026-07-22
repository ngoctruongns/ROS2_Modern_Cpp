// ============================================================================
// Topic 05 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
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

// Variadic template + fold expression (C++17): (... + args)
template<typename... Args>
auto sum(Args... args) {
  return (... + args);   // = ((a0 + a1) + a2) + ...
}

// sizeof...(pack) đếm số phần tử trong parameter pack.
template<typename... Args>
std::size_t count_args(Args... args) {
  (void)sizeof...(args);
  return sizeof...(args);
}

// Dispatch theo kiểu lúc BIÊN DỊCH bằng if constexpr + type traits.
template<typename T>
std::string describe() {
  if constexpr (std::is_integral_v<T>) {
    return "integral";
  } else if constexpr (std::is_floating_point_v<T>) {
    return "floating";
  } else {
    return "other";
  }
}

int main() {
  CHECK(sum(1, 2, 3) == 6);
  CHECK(sum(1, 2, 3, 4, 5) == 15);
  CHECK(sum(10) == 10);
  CHECK(sum(1.0, 2.0, 3.5) == 6.5);

  CHECK(count_args(1, 2, 3) == 3);
  CHECK(count_args() == 0);
  CHECK(count_args("a", 1, 2.0, 'x') == 4);

  CHECK(describe<int>() == "integral");
  CHECK(describe<long>() == "integral");
  CHECK(describe<bool>() == "integral");
  CHECK(describe<double>() == "floating");
  CHECK(describe<float>() == "floating");
  CHECK(describe<std::string>() == "other");

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
