// ============================================================================
// Topic 12 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// Observer / Pub-Sub với weak_ptr + tự prune.
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

template <typename T>
struct Observer {
  virtual void on_event(const T& value) = 0;
  virtual ~Observer() = default;
};

template <typename T>
class Subject {
public:
  void subscribe(const std::shared_ptr<Observer<T>>& obs) {
    observers_.emplace_back(obs);               // shared_ptr -> weak_ptr
  }

  size_t notify(const T& value) {
    std::vector<std::weak_ptr<Observer<T>>> alive;
    alive.reserve(observers_.size());
    size_t delivered = 0;
    for (auto& w : observers_) {
      if (auto s = w.lock()) {                  // observer còn sống?
        s->on_event(value);
        ++delivered;
        alive.push_back(w);                     // giữ lại
      }
      // else: hết hạn -> bỏ (prune)
    }
    observers_.swap(alive);                     // chỉ giữ những cái còn sống
    return delivered;
  }

  size_t tracked() const { return observers_.size(); }

private:
  std::vector<std::weak_ptr<Observer<T>>> observers_;
};

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

  size_t got = topic.notify(42);
  CHECK(got == 2);
  CHECK(a->last == 42 && a->count == 1);
  CHECK(b->last == 42 && b->count == 1);

  {
    auto c = std::make_shared<Recorder>("C");
    topic.subscribe(c);
    size_t got2 = topic.notify(7);
    CHECK(got2 == 3);
    CHECK(c->last == 7);
  }

  size_t got3 = topic.notify(100);
  CHECK(got3 == 2);
  CHECK(a->last == 100 && b->last == 100);
  CHECK(topic.tracked() == 2);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
