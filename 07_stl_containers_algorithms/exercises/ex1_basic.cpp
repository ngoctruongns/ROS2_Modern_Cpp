// ============================================================================
// Topic 07 — STL Containers & <algorithm> — Bài 1 (BASIC)
// Mục tiêu: std::vector<double>, std::unordered_map<std::string,int> (đếm),
//           duyệt bằng structured bindings, .at() vs [], find() vs count().
//
// Cách làm: điền code vào các chỗ có "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

int main() {
  // ---------------- Phần 1: std::vector<double> ----------------
  std::vector<double> v;

  // TODO 1: reserve trước 4 phần tử để tránh cấp phát lại (chỉ đổi capacity).
  //         Sau reserve, size vẫn phải == 0, capacity phải >= 4.
  // ...
  CHECK(v.size() == 0);
  CHECK(v.capacity() >= 4);

  // TODO 2: thêm lần lượt 4 giá trị 1.0, 2.0, 3.0, 4.0 vào v
  //         (dùng push_back hoặc emplace_back).
  // ...
  CHECK(v.size() == 4);
  CHECK(!v.empty() && v.front() == 1.0);
  CHECK(!v.empty() && v.back() == 4.0);

  // TODO 3: tính tổng bằng range-for (không dùng <numeric> ở bài này).
  double sum = 0.0;
  // for (...) ...
  CHECK(sum == 10.0);

  // .at() kiểm tra biên; ở đây phần tử index 2 phải là 3.0
  CHECK(v.size() > 2 && v.at(2) == 3.0);

  // ---------------- Phần 2: unordered_map đếm tần suất ----------------
  // Đếm số lần xuất hiện mỗi tên cảm biến.
  std::vector<std::string> readings = {"imu", "lidar", "imu", "gps", "imu", "lidar"};
  std::unordered_map<std::string, int> counts;

  // TODO 4: duyệt readings và tăng đếm cho mỗi tên.
  //         (operator[] trên khoá chưa có sẽ tự tạo giá trị mặc định 0 — tiện cho đếm.)
  // for (const auto& name : readings) ...
  (void)readings;

  CHECK(counts.size() == 3);         // imu, lidar, gps
  CHECK(counts.count("imu") && counts.at("imu") == 3);
  CHECK(counts.count("lidar") && counts.at("lidar") == 2);
  CHECK(counts.count("gps") && counts.at("gps") == 1);

  // TODO 5: dùng find() (KHÔNG dùng []) để kiểm tra khoá "imu" tồn tại,
  //         gán imu_found = true và imu_val = số đếm của "imu".
  bool imu_found = false;
  int  imu_val = 0;
  // auto it = counts.find("imu"); ...
  CHECK(imu_found);
  CHECK(imu_val == 3);

  // find() với khoá không tồn tại phải trả về end().
  CHECK(counts.find("camera") == counts.end());

  // TODO 6: tính tổng tất cả số đếm bằng structured bindings trong range-for.
  int total = 0;
  // for (const auto& [key, value] : counts) ...
  CHECK(total == 6);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
