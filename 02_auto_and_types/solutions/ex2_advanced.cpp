// ============================================================================
// Topic 02 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
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

// Structured binding trong range-for: [key, val] thay cho .first/.second.
int sum_values(const std::map<std::string, int>& m) {
  int sum = 0;
  for (const auto& [key, val] : m) {
    (void)key;
    sum += val;
  }
  return sum;
}

// Structured binding trên struct: tách các trường theo thứ tự khai báo.
double pose_total(const Pose& p) {
  const auto& [px, py, pth] = p;
  return px + py + pth;
}

// decltype(auto): giữ nguyên kiểu biểu thức v[i] -> int& (không copy).
decltype(auto) elem_ref(std::vector<int>& v, std::size_t i) {
  return v[i];
}

int main() {
  std::map<std::string, int> params{{"queue", 10}, {"rate", 30}, {"depth", 5}};
  CHECK(sum_values(params) == 45);

  int total_after = 0;
  for (auto& [key, val] : params) {   // auto& -> sửa value tại chỗ
    (void)key;
    val += 1;
    total_after += val;
  }
  CHECK(total_after == 48);

  Pose p{1.0, 2.0, 0.5};
  CHECK(pose_total(p) == 3.5);

  std::tuple<int, std::string, double> record{7, "imu", 1.5};
  auto [id, name, freq] = record;     // structured binding trên tuple
  CHECK(id == 7);
  CHECK(name == "imu");
  CHECK(freq == 1.5);

  std::vector<int> nums{10, 20, 30};
  CHECK((std::is_same<decltype(elem_ref(nums, 0)), int&>::value));
  elem_ref(nums, 0) = 11;             // sửa được vì trả về int&
  CHECK(nums[0] == 11);

  long sum = 0;
  for (const auto& n : nums) sum += n;  // const auto&: không copy
  CHECK(sum == 61);                     // 11 + 20 + 30

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
