// ============================================================================
// Topic 09 — Concurrency — Bài 1 (BASIC)
// Mục tiêu: std::thread (tạo/join), truyền tham số, data race, std::mutex +
//           std::lock_guard, std::atomic.
//
// Bối cảnh: N luồng cùng tăng MỘT bộ đếm chia sẻ, mỗi luồng K lần.
//           Kết quả ĐÚNG phải là N*K. Nếu không đồng bộ -> data race -> sai.
//
// Cách làm: điền code vào các chỗ "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

constexpr int N = 8;      // số luồng
constexpr int K = 100000; // mỗi luồng tăng K lần

// Trạng thái chia sẻ giữa các luồng.
struct Shared {
  long counter = 0;   // được nhiều luồng tăng
  std::mutex m;       // dùng để bảo vệ counter (khi bạn điền TODO)
};

// Hàm chạy trên mỗi luồng: tăng counter K lần.
//
// CÁCH SAI (chỉ để minh hoạ, ĐỪNG dùng): `++s.counter;` không khoá.
//   `++` gồm 3 bước đọc-tăng-ghi; hai luồng xen kẽ -> mất lượt tăng -> tổng < N*K.
//
// TODO 1: bảo vệ phép tăng bằng std::lock_guard<std::mutex> trên s.m
//         (mỗi vòng lặp khoá rồi ++). Kết quả sẽ luôn = N*K.
void increment_worker(Shared& s) {
  for (int i = 0; i < K; ++i) {
    std::lock_guard<std::mutex> lock(s.m);
    ++s.counter;   // TODO: thay bằng phiên bản có khoá (lock_guard)
  }
}

// Phiên bản dùng atomic (không cần mutex).
// TODO 2: tăng đúng K lần một std::atomic<long> bằng fetch_add / ++.
void atomic_worker(std::atomic<long>& n) {
  for (int i = 0; i < K; ++i) {
    ++n;   // TODO: tăng n một cách nguyên tử (vd ++n hoặc n.fetch_add(1))
  }
}

int main() {
  // --- Phần 1: bảo vệ bằng mutex ---
  Shared s;
  std::vector<std::thread> threads;

  // TODO 3: tạo N luồng, mỗi luồng chạy increment_worker(s).
  //         Truyền s BẰNG THAM CHIẾU: dùng std::ref(s).
    for (int i = 0; i < N; ++i)
        threads.emplace_back(increment_worker, std::ref(s));

  // TODO 4: join TẤT CẢ các luồng TRƯỚC KHI kiểm tra (bắt buộc, để xác định).
    for (auto& t : threads) t.join();

  // Sau khi mọi luồng đã join, kết quả phải xác định = N*K.
  CHECK(s.counter == static_cast<long>(N) * K);

  // --- Phần 2: bảo vệ bằng atomic ---
  std::atomic<long> n{0};
  std::vector<std::thread> athreads;

  // TODO 5: tạo N luồng chạy atomic_worker(n), rồi join hết trước khi CHECK.
    for (int i = 0; i < N; ++i)
        athreads.emplace_back(atomic_worker, std::ref(n));
    for (auto& t : athreads) t.join();

  CHECK(n.load() == static_cast<long>(N) * K);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
