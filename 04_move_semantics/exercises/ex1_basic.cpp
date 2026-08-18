// ============================================================================
// Topic 04 — Move Semantics — Bài 1 (BASIC)
// Mục tiêu: lvalue/rvalue, rvalue reference T&&, std::move chỉ là ép kiểu,
//           tự viết MOVE CONSTRUCTOR + MOVE ASSIGNMENT (cướp + null hoá nguồn).
//
// Lớp Buffer đếm số lần COPY và số lần MOVE bằng biến static. Nhiệm vụ của bạn:
// hoàn thiện move ctor & move assignment sao cho:
//   - move KHÔNG làm tăng bộ đếm copy,
//   - đối tượng nguồn trở nên RỖNG (empty) sau khi bị move.
//
// Cách làm: điền code vào các chỗ "TODO". Chạy tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <cstddef>
#include <iostream>
#include <utility>

// --- test harness nhỏ (giống mọi topic) ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// Buffer: bọc một mảng int cấp phát trên heap. Đếm copy vs move.
class Buffer {
public:
  // Cấp phát n phần tử, gán data_[i] = start + i.
  explicit Buffer(std::size_t n, int start = 0)
      : data_(new int[n]), size_(n) {
    for (std::size_t i = 0; i < n; ++i) data_[i] = start + static_cast<int>(i);
  }

  // Destructor: giải phóng tài nguyên (RAII).
  ~Buffer() { delete[] data_; }

  // Copy constructor: DEEP COPY -> tốn kém, tăng bộ đếm copy.
  Buffer(const Buffer& o) : data_(new int[o.size_]), size_(o.size_) {
    for (std::size_t i = 0; i < size_; ++i) data_[i] = o.data_[i];
    ++copies_;
  }

  // Copy assignment: DEEP COPY, tăng bộ đếm copy.
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

  // TODO 1: MOVE CONSTRUCTOR.
  //   - "Cướp" con trỏ data_ và size_ từ `o` (không cấp phát mới, không copy).
  //   - Vô hiệu hoá nguồn: o.data_ = nullptr; o.size_ = 0;
  //   - Tăng bộ đếm move: ++moves_;
  //   - Nhớ để `noexcept` (quan trọng cho std::vector khi grow).
  Buffer(Buffer&& o) noexcept {
    data_ = o.data_;   // <-- STUB: chưa cướp gì. Hãy thay bằng phần TODO ở trên.
    size_ = o.size_;
    o.data_ = nullptr;
    o.size_ = 0;
    ++moves_;
  }

  // TODO 2: MOVE ASSIGNMENT.
  //   - Chống self-move: if (this != &o) { ... }
  //   - Giải phóng tài nguyên hiện tại (delete[] data_) rồi cướp từ `o`.
  //   - Vô hiệu hoá nguồn và ++moves_.
  Buffer& operator=(Buffer&& o) noexcept {
    if (this != &o) {
        delete[] data_; // free old memory

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

  // Bộ đếm toàn cục.
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
  // --- copy làm tăng bộ đếm copy ---
  Buffer::reset();
  Buffer a(3, 100);                 // dựng thường, không copy/move
  CHECK(a.size() == 3);
  CHECK(a.at(2) == 102);

  Buffer b(a);                      // COPY constructor
  CHECK(Buffer::copies() == 1);
  CHECK(Buffer::moves() == 0);
  CHECK(b.at(1) == 101);

  // --- move constructor: không copy, nguồn rỗng ---
  Buffer::reset();
  Buffer c(std::move(a));           // std::move chỉ ép kiểu -> chọn MOVE ctor
  CHECK(Buffer::moves() == 1);      // đã move đúng 1 lần
  CHECK(Buffer::copies() == 0);     // move KHÔNG được copy
  CHECK(a.empty());                 // nguồn bị null hoá
  CHECK(c.size() == 3 && c.at(0) == 100);

  // --- move assignment ---
  Buffer::reset();
  Buffer d(2, 5);                   // d ban đầu có tài nguyên riêng
  d = std::move(b);                 // MOVE assignment
  CHECK(Buffer::moves() == 1);
  CHECK(Buffer::copies() == 0);
  CHECK(b.empty());                 // nguồn bị null hoá
  CHECK(d.size() == 3 && d.at(2) == 102);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
