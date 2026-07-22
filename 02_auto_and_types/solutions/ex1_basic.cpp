// ============================================================================
// Topic 02 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// `using` đọc thuận (trái->phải), hỗ trợ alias template — ưu tiên hơn typedef.
using IntVec = std::vector<int>;

// Hàm trả về auto: kiểu suy ra từ biểu thức `a + b` -> int.
auto add(int a, int b) {
  return a + b;
}

int main() {
  const int ci = 42;
  auto x = ci;          // auto bỏ top-level const -> x là `int`, bản copy
  x = 100;              // sửa được vì là copy độc lập
  CHECK(x == 100);
  CHECK(ci == 42);      // nguồn không đổi

  std::vector<int> v{10, 20, 30};

  auto& ref = v[0];     // auto& giữ tham chiếu -> bám vào v[0]
  ref = 99;
  CHECK(v[0] == 99);    // sửa qua ref thì nguồn đổi

  auto copy = v[1];     // auto -> bản copy độc lập
  copy = 0;
  CHECK(copy == 0);     // bản copy đã đổi
  CHECK(v[1] == 20);    // nhưng nguồn không đổi

  const auto& cref = v[2];  // đọc không copy
  CHECK(cref == 30);

  IntVec nums{1, 2, 3};
  CHECK(nums.size() == 3);
  CHECK((std::is_same<IntVec, std::vector<int>>::value));

  CHECK(add(2, 3) == 5);

  // decltype giữ đúng kiểu khai báo của biểu thức.
  CHECK((std::is_same<decltype(add(1, 1)), int>::value));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
