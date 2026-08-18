// ============================================================================
// Topic 05 — Templates & Generics — Bài 1 (BASIC)
// Mục tiêu: function template, class template, alias template, non-type param.
//
// Cách làm: điền code vào các chỗ có "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// ----------------------------------------------------------------------------
// TODO 1: function template `max_of`.
//   Trả về phần tử LỚN HƠN trong hai đối số cùng kiểu T.
//   Yêu cầu: dùng template<typename T>, so sánh bằng operator<.
// Hiện tại stub luôn trả về `a` -> một số CHECK sẽ FAIL cho tới khi bạn sửa.
template<typename T>
T max_of(T a, T b) {
  return (a < b) ? b : a; // TODO: trả về giá trị lớn hơn giữa a và b
}

// ----------------------------------------------------------------------------
// TODO 2: class template `Stack<T>` — ngăn xếp LIFO tổng quát.
//   Cần: push(const T&), pop() trả về phần tử vừa lấy, empty(), size().
//   Gợi ý: dùng std::vector<T> làm bộ chứa bên trong.
template<typename T>
class Stack {
  std::vector<T> data_;
public:
  void push(const T& v) {
    data_.push_back(v);
    // TODO: thêm v vào cuối data_
  }
  T pop() {
    T temp = data_.back();
    data_.pop_back();
    // TODO: lấy phần tử cuối, xoá nó khỏi data_, rồi trả về.
    return temp; // stub
  }
  bool empty() const {
    return data_.empty(); // TODO: data_ rỗng?
  }
  std::size_t size() const {
    return data_.size(); // TODO: số phần tử hiện có
  }
};

// ----------------------------------------------------------------------------
// TODO 3: alias template.
//   Định nghĩa `Vec<T>` là bí danh cho std::vector<T>.
template<typename T>
using Vec = std::vector<T>;

// ----------------------------------------------------------------------------
// TODO 4: non-type template parameter.
//   FixedArray<T, N> chứa mảng T[N]; hằng `size` = N (compile-time).
template<typename T, int N>
struct FixedArray {
  T data[N];
  static constexpr int size = N; // TODO: đặt bằng N
};

int main() {
  // --- function template ---
  CHECK(max_of(3, 7) == 7);            // T = int (deduced)
  CHECK(max_of(9, 2) == 9);
  CHECK(max_of(2.5, 1.5) == 2.5);      // T = double (deduced)
  CHECK(max_of<double>(1, 2.5) == 2.5); // explicit: ép T = double
  CHECK(max_of(std::string("abc"), std::string("abd")) == "abd");

  // --- class template ---
  Stack<int> si;
  CHECK(si.empty());
  si.push(10);
  si.push(20);
  CHECK(si.size() == 2);
  CHECK(!si.empty());
  CHECK(si.pop() == 20);   // LIFO
  CHECK(si.pop() == 10);
  CHECK(si.empty());

  Stack<std::string> ss;
  ss.push("a");
  ss.push("b");
  CHECK(ss.size() == 2);
  CHECK(ss.pop() == "b");

  // --- alias template ---
  // Bỏ comment sau khi định nghĩa Vec ở TODO 3:
  Vec<int> v = {1, 2, 3};
  CHECK(v.size() == 3);
  CHECK((std::is_same_v<Vec<int>, std::vector<int>>));

  // --- non-type template param ---
  FixedArray<float, 4> fa;
  CHECK((FixedArray<float, 4>::size == 4));
  CHECK(sizeof(fa.data) / sizeof(float) == 4);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
