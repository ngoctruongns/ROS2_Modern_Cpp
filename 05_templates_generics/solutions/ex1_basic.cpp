// ============================================================================
// Topic 05 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// Function template: một hàm, chạy cho mọi T có operator<.
template<typename T>
T max_of(T a, T b) {
  return (a < b) ? b : a;
}

// Class template: ngăn xếp LIFO tổng quát trên std::vector<T>.
template<typename T>
class Stack {
  std::vector<T> data_;
public:
  void push(const T& v) { data_.push_back(v); }
  T pop() {
    T v = data_.back();
    data_.pop_back();
    return v;
  }
  bool empty() const { return data_.empty(); }
  std::size_t size() const { return data_.size(); }
};

// Alias template: đặt tên gọn cho một kiểu template hoá.
template<typename T>
using Vec = std::vector<T>;

// Non-type template parameter: N là hằng compile-time.
template<typename T, int N>
struct FixedArray {
  T data[N];
  static constexpr int size = N;
};

int main() {
  CHECK(max_of(3, 7) == 7);
  CHECK(max_of(9, 2) == 9);
  CHECK(max_of(2.5, 1.5) == 2.5);
  CHECK(max_of<double>(1, 2.5) == 2.5);
  CHECK(max_of(std::string("abc"), std::string("abd")) == "abd");

  Stack<int> si;
  CHECK(si.empty());
  si.push(10);
  si.push(20);
  CHECK(si.size() == 2);
  CHECK(!si.empty());
  CHECK(si.pop() == 20);
  CHECK(si.pop() == 10);
  CHECK(si.empty());

  Stack<std::string> ss;
  ss.push("a");
  ss.push("b");
  CHECK(ss.size() == 2);
  CHECK(ss.pop() == "b");

  Vec<int> v = {1, 2, 3};
  CHECK(v.size() == 3);
  CHECK((std::is_same<Vec<int>, std::vector<int>>::value));

  FixedArray<float, 4> fa;
  CHECK((FixedArray<float, 4>::size == 4));
  CHECK(sizeof(fa.data) / sizeof(float) == 4);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
