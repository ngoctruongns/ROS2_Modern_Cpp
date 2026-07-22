// ============================================================================
// Topic 10 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// RAII + member-initializer list + explicit + in-class init + non-copyable.
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
  static int open_count;

  // explicit: chặn chuyển đổi ngầm từ std::string.
  // init-list gán name_ trước khi vào thân; thân ctor "acquire" (++count).
  explicit ScopedResource(const std::string& name) : name_(name) {
    ++open_count;
  }

  // destructor "release" (--count). Nhờ RAII, ra khỏi scope là tự chạy.
  ~ScopedResource() {
    --open_count;
  }

  // Non-copyable: cấm copy để mỗi resource chỉ có một chủ (giống handle độc quyền).
  ScopedResource(const ScopedResource&) = delete;
  ScopedResource& operator=(const ScopedResource&) = delete;

  // const member function: không sửa trạng thái, gọi được trên object const.
  const std::string& name() const { return name_; }

private:
  std::string name_ = "<unnamed>";   // in-class member initializer
};

int ScopedResource::open_count = 0;

static constexpr bool kCopyable =
    std::is_copy_constructible<ScopedResource>::value;

int main() {
  CHECK(ScopedResource::open_count == 0);

  {
    ScopedResource a("lidar");
    CHECK(ScopedResource::open_count == 1);
    CHECK(a.name() == "lidar");

    {
      ScopedResource b("imu");
      CHECK(ScopedResource::open_count == 2);
    }
    CHECK(ScopedResource::open_count == 1);
  }
  CHECK(ScopedResource::open_count == 0);

  CHECK(kCopyable == false);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
