// ============================================================================
// Topic 04 — Move Semantics — Bài 2 (ADVANCED)
// Mục tiêu:
//   - Perfect forwarding: factory make<T>(Args&&...) + std::forward.
//   - Rule of 5: lớp tài nguyên Res tự viết đủ 5 hàm đặc biệt.
//   - noexcept trên move ops, self-move-assignment an toàn.
//
// Ý tưởng kiểm tra: Res đếm số lần COPY vs MOVE. Nếu forwarding ĐÚNG thì:
//   - make<Res>(lvalue)  -> COPY (copies==1, moves==0)
//   - make<Res>(rvalue)  -> MOVE (moves==1, copies==0)
// Nếu quên std::forward, mọi thứ đều bị COPY.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
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

// Lớp tài nguyên theo Rule of 5 — đếm copy vs move.
class Res {
public:
  explicit Res(std::size_t n) : data_(new int[n]), size_(n) {
    for (std::size_t i = 0; i < n; ++i) data_[i] = static_cast<int>(i);
  }

  ~Res() { delete[] data_; }

  // Copy ctor (đã cho sẵn) — deep copy, đếm copy.
  Res(const Res& o) : data_(new int[o.size_]), size_(o.size_) {
    for (std::size_t i = 0; i < size_; ++i) data_[i] = o.data_[i];
    ++copies_;
  }

  // Copy assignment (đã cho sẵn).
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

  // TODO 1: MOVE CONSTRUCTOR (noexcept).
  //   Cướp data_/size_ từ `o`, null hoá `o`, và ++moves_.
  Res(Res&& o) noexcept {
    (void)o;
    data_ = nullptr;   // <-- STUB sai: hãy cướp tài nguyên từ o thay vì bỏ trống.
    size_ = 0;
  }

  // TODO 2: MOVE ASSIGNMENT (noexcept).
  //   PHẢI chống self-move: if (this != &o) { ... }
  //   Dọn tài nguyên cũ, cướp từ o, null hoá o, ++moves_.
  Res& operator=(Res&& o) noexcept {
    (void)o;
    // <-- STUB sai: chưa làm gì.
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

// TODO 3: FACTORY perfect-forwarding.
//   Dùng std::forward<Args>(args)... để GIỮ NGUYÊN l-value/r-value của từng đối số,
//   nhờ đó constructor của T chọn đúng copy hay move.
template <class T, class... Args>
T make(Args&&... args) {
  return T(args...);   // <-- STUB: quên std::forward -> mọi thứ đều COPY.
}

int main() {
  // --- lvalue -> COPY ---
  Res::reset();
  Res a(3);
  Res b = make<Res>(a);              // a là lvalue -> phải COPY
  CHECK(Res::copies() == 1);
  CHECK(Res::moves() == 0);
  CHECK(b.size() == 3 && b.at(2) == 2);

  // --- rvalue -> MOVE ---
  Res::reset();
  Res c = make<Res>(Res(4));         // đối số là rvalue -> phải MOVE
  CHECK(Res::moves() == 1);
  CHECK(Res::copies() == 0);
  CHECK(c.size() == 4 && c.at(3) == 3);

  // --- move assignment thường ---
  Res::reset();
  Res dst(1);
  Res src(5);
  dst = std::move(src);              // MOVE assignment
  CHECK(Res::moves() == 1);
  CHECK(Res::copies() == 0);
  CHECK(src.empty());               // nguồn bị null hoá
  CHECK(dst.size() == 5 && dst.at(4) == 4);

  // --- self-move-assignment phải AN TOÀN ---
  Res s(2);
  s = std::move(s);                  // nhờ guard (this != &o), s giữ nguyên & hợp lệ
  CHECK(s.size() == 2);
  CHECK(s.at(1) == 1);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
