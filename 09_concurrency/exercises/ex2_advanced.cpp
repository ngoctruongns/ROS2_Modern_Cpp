// ============================================================================
// Topic 09 — Concurrency — Bài 2 (ADVANCED)
// Mục tiêu: hàng đợi thông điệp thread-safe bằng std::mutex + std::unique_lock +
//           std::condition_variable (producer/consumer), và std::async + future.
//
// Bối cảnh: 1 producer đẩy M thông điệp, 1 consumer lấy đủ M thông điệp.
//           Consumer dùng cv.wait(...) để ngủ khi hàng đợi rỗng (không busy-wait).
//           Song song, dùng std::async tính tổng kỳ vọng rồi so khớp qua future.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
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

constexpr int M = 1000;   // số thông điệp

// --- Hàng đợi thread-safe ---
// CÁCH SAI (chỉ minh hoạ): dùng std::queue trần từ 2 luồng, hoặc consumer
// busy-wait `while (q.empty()) {}` không khoá -> data race + đốt CPU.
class TSQueue {
public:
  // TODO 1: khoá mutex, đẩy v vào q_, rồi notify_one() để đánh thức consumer.
  //   { std::lock_guard<std::mutex> lk(m_); q_.push(v); }
  //   cv_.notify_one();
  void push(int v) {
    (void)v;   // TODO
  }

  // TODO 2: dùng std::unique_lock; cv_.wait(lk, [&]{ return !q_.empty(); });
  //         rồi lấy front, pop, và trả về giá trị.
  int pop() {
    return -1;   // stub: KHÔNG chặn (để đề vẫn chạy tới khi bạn điền TODO)
  }

private:
  std::queue<int> q_;
  std::mutex m_;
  std::condition_variable cv_;
};

// Tính tổng 1..n (dùng cho std::async). Hàm thuần, không đụng trạng thái chia sẻ.
long sum_1_to_n(int n) {
  long s = 0;
  for (int i = 1; i <= n; ++i) s += i;
  return s;
}

int main() {
  TSQueue q;
  long consumed_sum = 0;
  int  consumed_cnt = 0;

  // Consumer: lấy đúng M thông điệp và cộng dồn.
  std::thread consumer([&] {
    for (int i = 0; i < M; ++i) {
      int v = q.pop();
      consumed_sum += v;
      ++consumed_cnt;
    }
  });

  // Producer: đẩy các giá trị 1..M.
  std::thread producer([&] {
    for (int i = 1; i <= M; ++i) q.push(i);
  });

  // TODO 3: (đồng thời) dùng std::async chạy sum_1_to_n(M) trên luồng riêng.
  //   std::future<long> fut = std::async(std::launch::async, sum_1_to_n, M);
  std::future<long> fut;   // TODO: khởi tạo bằng std::async như trên

  // BẮT BUỘC: join cả hai luồng TRƯỚC KHI kiểm tra (để kết quả xác định).
  producer.join();
  consumer.join();

  // TODO 4: lấy kết quả async bằng fut.get() (chặn tới khi xong).
  long async_sum = 0;   // TODO: = fut.get();
  (void)fut;

  const long expected = static_cast<long>(M) * (M + 1) / 2;   // tổng 1..M

  CHECK(consumed_cnt == M);              // đã tiêu thụ đủ M thông điệp
  CHECK(consumed_sum == expected);       // đúng nội dung
  CHECK(async_sum == expected);          // future trả đúng giá trị tính được

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
