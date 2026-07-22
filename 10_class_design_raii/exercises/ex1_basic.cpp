// ============================================================================
// Topic 10 — Class Design & RAII — Bài 1 (BASIC)
// Mục tiêu: RAII (acquire trong ctor / release trong dtor), member-initializer
//           list, explicit, in-class init, = delete (non-copyable), const method.
//
// Bối cảnh: ScopedResource mô phỏng một tài nguyên (vd handle file/thiết bị).
//           Một biến static đếm số resource ĐANG mở. Mở -> +1, huỷ -> -1.
//           Nhờ RAII, ra khỏi scope là tự động giải phóng.
//
// Cách làm: điền code vào các chỗ "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <string>
#include <type_traits>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

class ScopedResource {
public:
  // Đếm số resource đang mở (dùng chung cho mọi instance).
  static int open_count;

  // TODO 1: constructor `explicit` nhận tên resource.
  //   - Dùng member-initializer list để gán name_.
  //   - Trong THÂN constructor: tăng open_count (acquire).
  //   (Hiện tại đang là stub để file biên dịch được — hãy thay thế.)
  ScopedResource(const std::string& name) {
    (void)name;   // TODO: gán name_ qua init-list và ++open_count
  }

  // TODO 2: destructor — giảm open_count (release).
  //   (Stub rỗng để biên dịch được; hãy điền phần giảm count.)
  ~ScopedResource() {
    // TODO: --open_count;
  }

  // TODO 3: làm class NON-COPYABLE — cấm copy ctor và copy assignment (= delete).
  //   (Chưa cấm nên hiện tại vẫn copy được; CHECK non-copyable ở main sẽ nhắc bạn.)

  // TODO 4: `name()` là const member function trả về tên resource.
  const std::string& name() const {
    static const std::string empty;
    return empty;   // TODO: trả về name_
  }

private:
  std::string name_ = "<unnamed>";   // in-class member initializer
};

int ScopedResource::open_count = 0;

// Trait kiểm tra tại compile-time: class có bị cấm copy không?
// (Nếu bạn = delete copy ctor đúng cách, giá trị này sẽ là false.)
static constexpr bool kCopyable =
    std::is_copy_constructible<ScopedResource>::value;

int main() {
  CHECK(ScopedResource::open_count == 0);

  {
    ScopedResource a("lidar");
    CHECK(ScopedResource::open_count == 1);   // acquire trong ctor
    CHECK(a.name() == "lidar");

    {
      ScopedResource b("imu");
      CHECK(ScopedResource::open_count == 2);
    }
    CHECK(ScopedResource::open_count == 1);    // b ra khỏi scope -> release
  }
  CHECK(ScopedResource::open_count == 0);      // a ra khỏi scope -> release

  // ScopedResource phải là NON-COPYABLE (Rule: delete copy).
  CHECK(kCopyable == false);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
