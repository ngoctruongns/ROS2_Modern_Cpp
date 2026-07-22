// ============================================================================
// Topic 02 — auto & Type Deduction — Bài 1 (BASIC)
// Mục tiêu: quy tắc suy luận auto (copy vs tham chiếu), auto& / const auto&,
//           type alias bằng `using`, và decltype.
//
// Cách làm: điền code vào các chỗ có "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// TODO 1: Tạo type alias bằng `using` cho một vector các int, tên là `IntVec`.
//         (Đừng dùng typedef — Modern C++ ưu tiên `using`.)
using IntVec = std::vector<double>; // TODO: sửa cho đúng thành std::vector<int>

// TODO 2: Viết hàm `add` trả về `auto` (C++14) cộng hai int.
//         Kiểu trả về được suy ra từ biểu thức return.
auto add(int a, int b) {
  (void)a; (void)b;
  return 0; // TODO: trả về a + b
}

int main() {
  // --- auto bỏ top-level const/ref: cho ra bản COPY ---
  const int ci = 42;
  auto x = ci;          // x là `int` (const bị bỏ) -> sửa được
  // TODO 3: chứng minh x là bản copy: đổi x thành 100 rồi kiểm tra ci KHÔNG đổi.
  // ...                // gán x = 100;
  CHECK(x == 100);
  CHECK(ci == 42);      // ci không bị ảnh hưởng vì x là copy

  // --- auto& bám vào nguồn: sửa qua tham chiếu thì nguồn đổi ---
  std::vector<int> v{10, 20, 30};

  // TODO 4: tạo `ref` là tham chiếu tới v[0] bằng auto&, rồi gán ref = 99.
  //         Sau đó v[0] phải bằng 99.
  // auto& ref = ...;
  // ref = 99;
  CHECK(v[0] == 99);

  // TODO 5: tạo `copy` là BẢN COPY của v[1] bằng auto, rồi đổi copy = 0.
  //         v[1] KHÔNG được đổi (vẫn 20) vì copy là bản sao.
  // auto copy = ...;
  // copy = 0;
  CHECK(v[1] == 20);

  // --- const auto& để đọc không copy ---
  const auto& cref = v[2];
  CHECK(cref == 30);

  // --- type alias IntVec: khẳng định đúng kiểu (chạy lúc runtime) ---
  IntVec nums{1, 2, 3};
  CHECK(nums.size() == 3);
  CHECK((std::is_same<IntVec, std::vector<int>>::value));

  // --- hàm trả về auto ---
  CHECK(add(2, 3) == 5);

  // --- decltype giữ đúng kiểu khai báo ---
  // TODO 6: dùng CHECK + decltype để khẳng định add(1,1) có kiểu int.
  // CHECK((std::is_same<decltype(add(1, 1)), int>::value));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
