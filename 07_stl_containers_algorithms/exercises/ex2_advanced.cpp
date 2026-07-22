// ============================================================================
// Topic 07 — STL Containers & <algorithm> — Bài 2 (ADVANCED)
// Bối cảnh: mô phỏng một pipeline xử lý dữ liệu cảm biến (vd tia laser/range)
//           nằm trong std::vector<double>:
//   1) Lọc bỏ giá trị ngoài tầm [0, 100] bằng erase–remove idiom.
//   2) Tính trung bình bằng std::accumulate (<numeric>).
//   3) Tìm giá trị lớn nhất bằng std::max_element.
//   4) Đếm số giá trị "gần" (< 10) bằng std::count_if.
//   5) Sắp xếp giảm dần bằng std::sort + lambda.
//   6) Scale toàn bộ (x2) bằng std::transform.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// So sánh số thực trong sai số nhỏ (an toàn hơn == với double).
static bool almost(double a, double b, double eps = 1e-9) {
  return std::fabs(a - b) < eps;
}

// TODO 1: lọc bỏ mọi phần tử NGOÀI [lo, hi] khỏi v, DÙNG erase–remove idiom.
//         Gợi ý: v.erase(std::remove_if(v.begin(), v.end(), pred), v.end());
void filter_range(std::vector<double>& v, double lo, double hi) {
  (void)v; (void)lo; (void)hi;
  // TODO
}

// TODO 2: trả về trung bình cộng của v dùng std::accumulate (hạt giống 0.0).
//         Vector rỗng -> trả 0.0.
double mean(const std::vector<double>& v) {
  (void)v;
  return 0.0; // TODO
}

int main() {
  // -1.0 và 150.0 là giá trị lỗi/ngoài tầm cần loại bỏ.
  std::vector<double> data = {5.0, -1.0, 20.0, 150.0, 8.0, 42.0, 3.0};

  // --- 1) Lọc ngoài tầm [0, 100] ---
  filter_range(data, 0.0, 100.0);
  CHECK(data.size() == 5);                       // còn: 5,20,8,42,3
  CHECK(std::find(data.begin(), data.end(), -1.0) == data.end());
  CHECK(std::find(data.begin(), data.end(), 150.0) == data.end());

  // --- 2) Trung bình ---  (5+20+8+42+3)/5 = 15.6
  CHECK(almost(mean(data), 15.6));

  // --- 3) Max element (trả iterator, nhớ deref) ---
  // TODO 3: dùng std::max_element gán max_val = giá trị lớn nhất.
  double max_val = 0.0;
  CHECK(almost(max_val, 42.0));

  // --- 4) Đếm số giá trị "gần" (< 10) ---
  // TODO 4: dùng std::count_if đếm số phần tử < 10.0.
  long near_count = 0;
  CHECK(near_count == 3);                         // 5, 8, 3

  // --- 5) Sắp xếp GIẢM DẦN bằng lambda ---
  // TODO 5: std::sort với comparator giảm dần.
  // ...
  CHECK(data.front() == 42.0);
  CHECK(data.back() == 3.0);
  CHECK(std::is_sorted(data.begin(), data.end(),
                       [](double a, double b){ return a > b; }));

  // --- 6) Scale x2 tại chỗ bằng transform ---
  // TODO 6: nhân đôi mọi phần tử của data bằng std::transform.
  // ...
  CHECK(almost(data.front(), 84.0));
  CHECK(almost(mean(data), 31.2));                // trung bình cũng gấp đôi

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
