// ============================================================================
// Topic 07 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

int main() {
  // ---------------- Phần 1: std::vector<double> ----------------
  std::vector<double> v;

  v.reserve(4);                 // chỉ đổi capacity, size vẫn 0
  CHECK(v.size() == 0);
  CHECK(v.capacity() >= 4);

  v.push_back(1.0);
  v.push_back(2.0);
  v.emplace_back(3.0);          // dựng tại chỗ
  v.emplace_back(4.0);
  CHECK(v.size() == 4);
  CHECK(!v.empty() && v.front() == 1.0);
  CHECK(!v.empty() && v.back() == 4.0);

  double sum = 0.0;
  for (double x : v) sum += x;
  CHECK(sum == 10.0);

  CHECK(v.size() > 2 && v.at(2) == 3.0);

  // ---------------- Phần 2: unordered_map đếm tần suất ----------------
  std::vector<std::string> readings = {"imu", "lidar", "imu", "gps", "imu", "lidar"};
  std::unordered_map<std::string, int> counts;

  for (const auto& name : readings) {
    ++counts[name];             // [] tạo mặc định 0 nếu chưa có, rồi tăng
  }

  CHECK(counts.size() == 3);
  CHECK(counts.count("imu") && counts.at("imu") == 3);
  CHECK(counts.count("lidar") && counts.at("lidar") == 2);
  CHECK(counts.count("gps") && counts.at("gps") == 1);

  bool imu_found = false;
  int  imu_val = 0;
  auto it = counts.find("imu");     // find: không chèn nếu vắng mặt
  if (it != counts.end()) {
    imu_found = true;
    imu_val = it->second;
  }
  CHECK(imu_found);
  CHECK(imu_val == 3);

  CHECK(counts.find("camera") == counts.end());

  int total = 0;
  for (const auto& [key, value] : counts) {   // structured bindings (C++17)
    (void)key;
    total += value;
  }
  CHECK(total == 6);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
