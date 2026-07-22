// ============================================================================
// Topic 04 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
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

class Buffer {
public:
  explicit Buffer(std::size_t n, int start = 0)
      : data_(new int[n]), size_(n) {
    for (std::size_t i = 0; i < n; ++i) data_[i] = start + static_cast<int>(i);
  }

  ~Buffer() { delete[] data_; }

  Buffer(const Buffer& o) : data_(new int[o.size_]), size_(o.size_) {
    for (std::size_t i = 0; i < size_; ++i) data_[i] = o.data_[i];
    ++copies_;
  }

  Buffer& operator=(const Buffer& o) {
    if (this != &o) {
      delete[] data_;
      data_ = new int[o.size_];
      size_ = o.size_;
      for (std::size_t i = 0; i < size_; ++i) data_[i] = o.data_[i];
      ++copies_;
    }
    return *this;
  }

  // MOVE CONSTRUCTOR: cướp con trỏ, không cấp phát/copy, null hoá nguồn.
  Buffer(Buffer&& o) noexcept : data_(o.data_), size_(o.size_) {
    o.data_ = nullptr;   // nguồn không còn sở hữu -> không double free
    o.size_ = 0;
    ++moves_;
  }

  // MOVE ASSIGNMENT: chống self-move, dọn tài nguyên cũ, rồi cướp.
  Buffer& operator=(Buffer&& o) noexcept {
    if (this != &o) {
      delete[] data_;      // dọn tài nguyên hiện tại
      data_ = o.data_;     // cướp
      size_ = o.size_;
      o.data_ = nullptr;   // null hoá nguồn
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

long Buffer::copies_ = 0;
long Buffer::moves_  = 0;

int main() {
  Buffer::reset();
  Buffer a(3, 100);
  CHECK(a.size() == 3);
  CHECK(a.at(2) == 102);

  Buffer b(a);
  CHECK(Buffer::copies() == 1);
  CHECK(Buffer::moves() == 0);
  CHECK(b.at(1) == 101);

  Buffer::reset();
  Buffer c(std::move(a));
  CHECK(Buffer::moves() == 1);
  CHECK(Buffer::copies() == 0);
  CHECK(a.empty());
  CHECK(c.size() == 3 && c.at(0) == 100);

  Buffer::reset();
  Buffer d(2, 5);
  d = std::move(b);
  CHECK(Buffer::moves() == 1);
  CHECK(Buffer::copies() == 0);
  CHECK(b.empty());
  CHECK(d.size() == 3 && d.at(2) == 102);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
