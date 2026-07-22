// ============================================================================
// Topic 09 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// Hàng đợi thread-safe (mutex + condition_variable) + std::async/future.
// ============================================================================
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

constexpr int M = 1000;

class TSQueue {
public:
  void push(int v) {
    {
      std::lock_guard<std::mutex> lk(m_);   // khoá khi sửa hàng đợi
      q_.push(v);
    }
    cv_.notify_one();                       // đánh thức consumer đang wait
  }

  int pop() {
    std::unique_lock<std::mutex> lk(m_);            // unique_lock: cần cho CV
    cv_.wait(lk, [&] { return !q_.empty(); });      // ngủ tới khi có hàng (chống spurious)
    int v = q_.front();
    q_.pop();
    return v;
  }

private:
  std::queue<int> q_;
  std::mutex m_;
  std::condition_variable cv_;
};

long sum_1_to_n(int n) {
  long s = 0;
  for (int i = 1; i <= n; ++i) s += i;
  return s;
}

int main() {
  TSQueue q;
  long consumed_sum = 0;
  int  consumed_cnt = 0;

  std::thread consumer([&] {
    for (int i = 0; i < M; ++i) {
      int v = q.pop();
      consumed_sum += v;
      ++consumed_cnt;
    }
  });

  std::thread producer([&] {
    for (int i = 1; i <= M; ++i) q.push(i);
  });

  // Chạy song song một phép tính trên luồng riêng, lấy kết quả qua future.
  std::future<long> fut = std::async(std::launch::async, sum_1_to_n, M);

  producer.join();      // join TRƯỚC khi CHECK -> xác định
  consumer.join();

  long async_sum = fut.get();   // chặn tới khi async xong

  const long expected = static_cast<long>(M) * (M + 1) / 2;

  CHECK(consumed_cnt == M);
  CHECK(consumed_sum == expected);
  CHECK(async_sum == expected);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
