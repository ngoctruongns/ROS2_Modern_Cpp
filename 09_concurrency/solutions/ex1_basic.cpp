// ============================================================================
// Topic 09 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// std::thread, std::ref, std::mutex + lock_guard, std::atomic.
// ============================================================================
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

constexpr int N = 8;
constexpr int K = 100000;

struct Shared {
  long counter = 0;
  std::mutex m;
};

// Bảo vệ phép tăng bằng lock_guard: mỗi vòng khoá -> ++ -> tự mở khoá.
void increment_worker(Shared& s) {
  for (int i = 0; i < K; ++i) {
    std::lock_guard<std::mutex> lk(s.m);   // RAII: tự mở khoá cuối scope
    ++s.counter;
  }
}

// atomic: ++ nguyên tử, không cần mutex.
void atomic_worker(std::atomic<long>& n) {
  for (int i = 0; i < K; ++i) {
    n.fetch_add(1);   // tương đương ++n
  }
}

int main() {
  // --- Phần 1: mutex ---
  Shared s;
  std::vector<std::thread> threads;
  for (int i = 0; i < N; ++i)
    threads.emplace_back(increment_worker, std::ref(s));   // std::ref: tham chiếu thật
  for (auto& t : threads) t.join();                        // join hết TRƯỚC khi CHECK

  CHECK(s.counter == static_cast<long>(N) * K);

  // --- Phần 2: atomic ---
  std::atomic<long> n{0};
  std::vector<std::thread> athreads;
  for (int i = 0; i < N; ++i)
    athreads.emplace_back(atomic_worker, std::ref(n));
  for (auto& t : athreads) t.join();

  CHECK(n.load() == static_cast<long>(N) * K);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
