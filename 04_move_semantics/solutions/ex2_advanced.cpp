// ============================================================================
// Topic 04 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// ============================================================================
#include <cstddef>
#include <iostream>
#include <utility>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

class Res {
public:
  explicit Res(std::size_t n) : data_(new int[n]), size_(n) {
    for (std::size_t i = 0; i < n; ++i) data_[i] = static_cast<int>(i);
  }

  ~Res() { delete[] data_; }

  Res(const Res& o) : data_(new int[o.size_]), size_(o.size_) {
    for (std::size_t i = 0; i < size_; ++i) data_[i] = o.data_[i];
    ++copies_;
  }

  Res& operator=(const Res& o) {
    if (this != &o) {
      delete[] data_;
      data_ = new int[o.size_];
      size_ = o.size_;
      for (std::size_t i = 0; i < size_; ++i) data_[i] = o.data_[i];
      ++copies_;
    }
    return *this;
  }

  // MOVE CONSTRUCTOR — noexcept giúp std::vector move (thay vì copy) khi grow.
  Res(Res&& o) noexcept : data_(o.data_), size_(o.size_) {
    o.data_ = nullptr;
    o.size_ = 0;
    ++moves_;
  }

  // MOVE ASSIGNMENT — có guard chống self-move.
  Res& operator=(Res&& o) noexcept {
    if (this != &o) {
      delete[] data_;
      data_ = o.data_;
      size_ = o.size_;
      o.data_ = nullptr;
      o.size_ = 0;
      ++moves_;
    }
    return *this;
  }

  std::size_t size() const { return size_; }
  bool empty() const { return data_ == nullptr; }
  int at(std::size_t i) const { return data_[i]; }

  static long copies() { return copies_; }
  static long moves()  { return moves_;  }
  static void reset()  { copies_ = 0; moves_ = 0; }

private:
  int* data_ = nullptr;
  std::size_t size_ = 0;
  static long copies_;
  static long moves_;
};

long Res::copies_ = 0;
long Res::moves_  = 0;

// FACTORY perfect-forwarding: std::forward giữ đúng l/r-value của mỗi đối số.
template <class T, class... Args>
T make(Args&&... args) {
  return T(std::forward<Args>(args)...);
}

int main() {
  Res::reset();
  Res a(3);
  Res b = make<Res>(a);              // lvalue -> COPY
  CHECK(Res::copies() == 1);
  CHECK(Res::moves() == 0);
  CHECK(b.size() == 3 && b.at(2) == 2);

  Res::reset();
  Res c = make<Res>(Res(4));         // rvalue -> MOVE
  CHECK(Res::moves() == 1);
  CHECK(Res::copies() == 0);
  CHECK(c.size() == 4 && c.at(3) == 3);

  Res::reset();
  Res dst(1);
  Res src(5);
  dst = std::move(src);
  CHECK(Res::moves() == 1);
  CHECK(Res::copies() == 0);
  CHECK(src.empty());
  CHECK(dst.size() == 5 && dst.at(4) == 4);

  Res s(2);
  s = std::move(s);                  // self-move an toàn nhờ guard
  CHECK(s.size() == 2);
  CHECK(s.at(1) == 1);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
