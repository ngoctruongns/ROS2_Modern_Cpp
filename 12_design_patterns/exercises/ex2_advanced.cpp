// ============================================================================
// Topic 12 — Design Patterns — Bài 2 (ADVANCED)
// Mục tiêu: OBSERVER / PUB-SUB làm ĐÚNG với weak_ptr (liên hệ topic 01).
//   - Subject<T> (giống một "topic") giữ std::weak_ptr tới các observer.
//   - notify(const T&) BỎ QUA observer đã bị huỷ và TỰ PRUNE weak_ptr hết hạn.
//   -> không bao giờ dangling: Subject KHÔNG giữ observer sống mãi.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <memory>
#include <string>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// ---- Interface Observer<T>: nhận sự kiện kiểu T ----
template <typename T>
struct Observer {
  virtual void on_event(const T& value) = 0;
  virtual ~Observer() = default;
};

// ---- Subject<T>: giữ weak_ptr tới observer, phát sự kiện, tự prune ----
template <typename T>
class Subject {
public:
  // subscribe: lưu weak_ptr (KHÔNG sở hữu observer).
  void subscribe(const std::shared_ptr<Observer<T>>& obs) {
    // TODO 1: thêm obs (dưới dạng weak_ptr) vào observers_.
    (void)obs;
    // TODO
  }

  // notify: gọi on_event cho observer còn sống, đồng thời loại bỏ weak_ptr hết hạn.
  // Trả về SỐ observer thực sự nhận được sự kiện.
  size_t notify(const T& value) {
    (void)value;
    // TODO 2: duyệt observers_. Với mỗi w:
    //   - nếu auto s = w.lock() thành công -> s->on_event(value), đếm +1, giữ lại w.
    //   - nếu w.expired() -> BỎ (prune, không giữ lại).
    // Gợi ý: dựng một vector mới chỉ chứa các weak_ptr còn sống rồi swap.
    return 0;   // TODO
  }

  // Số observer đang được theo dõi (kể cả chưa prune). Dùng để kiểm tra prune.
  size_t tracked() const { return observers_.size(); }

private:
  std::vector<std::weak_ptr<Observer<T>>> observers_;
};

// ---- Một observer cụ thể: ghi lại giá trị nhận được ----
struct Recorder : Observer<int> {
  std::string name;
  int last = -1;
  int count = 0;
  explicit Recorder(std::string n) : name(std::move(n)) {}
  void on_event(const int& value) override {
    last = value;
    ++count;
  }
};

int main() {
  Subject<int> topic;

  auto a = std::make_shared<Recorder>("A");
  auto b = std::make_shared<Recorder>("B");
  topic.subscribe(a);
  topic.subscribe(b);

  // --- Cả hai observer đều nhận được sự kiện ---
  size_t got = topic.notify(42);
  CHECK(got == 2);
  CHECK(a->last == 42 && a->count == 1);
  CHECK(b->last == 42 && b->count == 1);

  // --- Thêm một observer trong scope rồi cho nó chết ---
  {
    auto c = std::make_shared<Recorder>("C");
    topic.subscribe(c);
    size_t got2 = topic.notify(7);
    CHECK(got2 == 3);                 // A, B, C đều nhận
    CHECK(c->last == 7);
  }   // c bị huỷ ở đây -> weak_ptr trong Subject hết hạn

  // --- Sau khi C chết: notify chỉ giao cho A, B và TỰ PRUNE weak_ptr của C ---
  size_t got3 = topic.notify(100);
  CHECK(got3 == 2);                   // chỉ A, B còn sống
  CHECK(a->last == 100 && b->last == 100);
  CHECK(topic.tracked() == 2);        // weak_ptr hết hạn đã bị loại bỏ (no dangling)

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
