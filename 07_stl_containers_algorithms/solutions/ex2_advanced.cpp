// ============================================================================
// Topic 07 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
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

static bool almost(double a, double b, double eps = 1e-9) {
  return std::fabs(a - b) < eps;
}

// erase–remove idiom: remove_if dồn phần tử GIỮ LẠI lên đầu, trả iterator "đuôi rác";
// vector::erase cắt phần đuôi đó đi. Thiếu erase là lỗi kinh điển.
void filter_range(std::vector<double>& v, double lo, double hi) {
  v.erase(std::remove_if(v.begin(), v.end(),
                         [lo, hi](double x){ return x < lo || x > hi; }),
          v.end());
}

// accumulate với hạt giống 0.0 (double) để không bị làm tròn về int.
double mean(const std::vector<double>& v) {
  if (v.empty()) return 0.0;
  double sum = std::accumulate(v.begin(), v.end(), 0.0);
  return sum / static_cast<double>(v.size());
}

int main() {
  std::vector<double> data = {5.0, -1.0, 20.0, 150.0, 8.0, 42.0, 3.0};

  // --- 1) Lọc ngoài tầm [0, 100] ---
  filter_range(data, 0.0, 100.0);
  CHECK(data.size() == 5);
  CHECK(std::find(data.begin(), data.end(), -1.0) == data.end());
  CHECK(std::find(data.begin(), data.end(), 150.0) == data.end());

  // --- 2) Trung bình ---
  CHECK(almost(mean(data), 15.6));

  // --- 3) Max element ---
  double max_val = 0.0;
  auto max_it = std::max_element(data.begin(), data.end());
  if (max_it != data.end()) max_val = *max_it;
  CHECK(almost(max_val, 42.0));

  // --- 4) Đếm giá trị < 10 ---
  long near_count = std::count_if(data.begin(), data.end(),
                                  [](double x){ return x < 10.0; });
  CHECK(near_count == 3);

  // --- 5) Sắp xếp giảm dần ---
  std::sort(data.begin(), data.end(),
            [](double a, double b){ return a > b; });
  CHECK(data.front() == 42.0);
  CHECK(data.back() == 3.0);
  CHECK(std::is_sorted(data.begin(), data.end(),
                       [](double a, double b){ return a > b; }));

  // --- 6) Scale x2 tại chỗ ---
  std::transform(data.begin(), data.end(), data.begin(),
                 [](double x){ return x * 2.0; });
  CHECK(almost(data.front(), 84.0));
  CHECK(almost(mean(data), 31.2));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
