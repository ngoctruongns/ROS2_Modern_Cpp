// ============================================================================
// Topic 02 — auto & Type Deduction — Bài 2 (ADVANCED)
// Mục tiêu: structured bindings (C++17) trên map / struct / tuple,
//           decltype(auto), và range-for bằng const auto&.
// Bối cảnh: xử lý một "bảng tham số" kiểu std::map (giống parameter map của ROS 2).
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

struct Pose {
  double x, y, theta;
};

// TODO 1: cộng TẤT CẢ giá trị trong map bằng range-for + structured binding.
//         Gợi ý: for (const auto& [key, val] : m) sum += val;
int sum_values(const std::map<std::string, int>& m) {
  (void)m;
  return 0; // TODO
}

// TODO 2: dùng structured binding trên struct Pose để trả về x + y + theta.
//         Gợi ý: const auto& [px, py, pth] = p;
double pose_total(const Pose& p) {
  (void)p;
  return 0.0; // TODO
}

// TODO 3: đổi kiểu trả về thành decltype(auto) để trả về THAM CHIẾU tới v[i]
//         (nếu để `auto` thì trả về bản copy int, không sửa được nguồn).
auto elem_ref(std::vector<int>& v, std::size_t i) {
  return v[i];
}

int main() {
  // --- structured binding trên map ---
  std::map<std::string, int> params{{"queue", 10}, {"rate", 30}, {"depth", 5}};
  CHECK(sum_values(params) == 45);

  // --- structured binding: sửa value tại chỗ bằng auto& ---
  int total_after = 0;
  for (auto& [key, val] : params) {   // auto& -> sửa được value
    (void)key;
    val += 1;                          // tăng mỗi tham số lên 1
    total_after += val;
  }
  CHECK(total_after == 48);            // 45 + 3 phần tử

  // --- structured binding trên struct ---
  Pose p{1.0, 2.0, 0.5};
  CHECK(pose_total(p) == 3.5);

  // --- structured binding trên tuple ---
  std::tuple<int, std::string, double> record{7, "imu", 1.5};
  (void)record;
  // TODO 4: tách record thành [id, name, freq] bằng structured binding.
  //         Xoá dòng (void)record; và 3 dòng khai báo dưới đây,
  //         thay bằng: auto [id, name, freq] = record;
  int id = 0;
  std::string name = "";
  double freq = 0.0;
  CHECK(id == 7);
  CHECK(name == "imu");
  CHECK(freq == 1.5);

  // --- decltype(auto) giữ tham chiếu ---
  std::vector<int> nums{10, 20, 30};
  CHECK((std::is_same<decltype(elem_ref(nums, 0)), int&>::value));

  // --- range-for bằng const auto& (không copy) để tính tổng ---
  long sum = 0;
  // TODO 5: duyệt nums bằng `for (const auto& n : nums)` và cộng vào sum.
  // ...
  CHECK(sum == 60);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
