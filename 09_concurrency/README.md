# 09 — Concurrency (Đa luồng: thread, mutex, atomic, condition_variable, async)

> **Vì sao quan trọng cho ROS 2?** `MultiThreadedExecutor` chạy callback trên nhiều luồng
> cùng lúc. Ngay khi hai callback đọc/ghi *chung* một biến, bạn đã bước vào thế giới
> **data race**. Hiểu `std::thread`, `mutex`, `atomic`, `condition_variable` là điều kiện
> để viết node đa luồng không sập ngẫu nhiên.

---

## A. Basic

### 1. `std::thread` — tạo & chờ / create & join
- Khởi tạo là **chạy ngay** một hàm trên luồng mới.
- **Bắt buộc** `join()` (chờ xong) hoặc `detach()` (thả trôi) **trước khi** đối tượng
  `std::thread` bị huỷ — nếu không, destructor gọi `std::terminate()` → crash.
- `join()` là cách an toàn & phổ biến: chặn luồng hiện tại tới khi luồng con kết thúc.

```cpp
std::thread t([]{ std::cout << "hello from thread\n"; });
t.join();                 // chờ xong; hoặc t.detach() để chạy nền
```

### 2. Truyền tham số / Passing args
- Tham số được **copy vào** luồng. Muốn truyền tham chiếu phải bọc `std::ref`.

```cpp
void work(int id, int& out);
int r = 0;
std::thread t(work, 7, std::ref(r));   // std::ref để nhận tham chiếu thật
t.join();
```

### 3. Data race & `std::mutex` + `std::lock_guard`
- **Data race**: ≥ 2 luồng truy cập cùng ô nhớ, ≥ 1 là ghi, không đồng bộ → *hành vi
  không xác định* (kết quả sai, khác nhau mỗi lần chạy).
- `std::mutex` cho phép **một** luồng vào vùng tới hạn (critical section) tại một thời điểm.
- `std::lock_guard` = RAII: khoá khi tạo, **tự mở khoá** khi ra khỏi scope (kể cả khi ném
  exception). Đừng bao giờ `lock()/unlock()` bằng tay nếu tránh được.

```cpp
std::mutex m;
long counter = 0;
{
  std::lock_guard<std::mutex> lk(m);   // khoá
  ++counter;                           // vùng tới hạn
}                                      // tự mở khoá ở đây
```

### 4. `std::scoped_lock` (C++17)
- Giống `lock_guard` nhưng khoá **nhiều mutex cùng lúc** một cách an toàn, tránh deadlock
  (dùng thuật toán tránh chết khoá bên trong).

```cpp
std::scoped_lock lk(m1, m2);   // khoá cả hai, không lo thứ tự
```

---

## B. Nâng cao / Advanced

### 5. `std::atomic<T>` — thao tác nguyên tử / lock-free
- Với kiểu số nguyên/pointer, `atomic` cho phép tăng/giảm/so-sánh-đổi **không cần mutex**.
- `++a`, `a.fetch_add(1)` là nguyên tử → an toàn cho **bộ đếm** chia sẻ.
- **Memory ordering** (nâng cao): mặc định `memory_order_seq_cst` (mạnh nhất, dễ đúng nhất).
  Chỉ nới lỏng (`relaxed`, `acquire`/`release`) khi thật sự hiểu và cần hiệu năng.

```cpp
std::atomic<long> n{0};
n.fetch_add(1);            // ++ nguyên tử, không cần khoá
long v = n.load();
```

### 6. `std::unique_lock` + `std::condition_variable` — producer/consumer
- `unique_lock` linh hoạt hơn `lock_guard`: mở/khoá lại được → **bắt buộc** cho CV.
- `condition_variable::wait(lock, pred)` mở khoá & ngủ tới khi được `notify_*` **và** `pred`
  đúng; tỉnh dậy thì khoá lại. `pred` chống **spurious wakeup** (tỉnh giả).
- Producer đẩy dữ liệu rồi `notify_one()`; consumer `wait` tới khi có hàng.

```cpp
std::mutex m; std::condition_variable cv; std::queue<int> q;
// producer:
{ std::lock_guard<std::mutex> lk(m); q.push(x); } cv.notify_one();
// consumer:
std::unique_lock<std::mutex> lk(m);
cv.wait(lk, [&]{ return !q.empty(); });   // ngủ tới khi có hàng
int v = q.front(); q.pop();
```

### 7. `std::async` + `std::future` + `.get()`
- `std::async` chạy một hàm (có thể trên luồng khác) và trả `future<T>`.
- `.get()` **chặn** tới khi có kết quả rồi trả về — tự đồng bộ, không cần join thủ công.
- Dùng `std::launch::async` để ép chạy trên luồng riêng (mặc định có thể lười).

```cpp
std::future<int> f = std::async(std::launch::async, []{ return 6*7; });
int answer = f.get();     // chặn tới khi xong -> 42
```

### 8. Tránh deadlock / Deadlock avoidance
- **Deadlock** kinh điển: luồng A khoá m1 rồi chờ m2, luồng B khoá m2 rồi chờ m1.
- Cách tránh: (1) **luôn khoá theo cùng một thứ tự**, hoặc (2) dùng `std::scoped_lock(m1, m2)`
  khoá tất cả cùng lúc.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
rclcpp::executors::MultiThreadedExecutor exec;   // nhiều luồng chạy callback
exec.add_node(node);
exec.spin();
```
- **`MultiThreadedExecutor`**: callback có thể chạy song song → mọi state chia sẻ giữa các
  callback phải bảo vệ bằng `mutex` hoặc `atomic`.
- **Callback groups**:
  - `MutuallyExclusive` (mặc định): các callback trong nhóm **không** chạy đồng thời →
    an toàn hơn, ít cần khoá.
  - `Reentrant`: cho phép callback trong nhóm chạy song song → **bạn** phải tự đồng bộ state.
- **Spin ở luồng nền**: `std::thread spin_thread([&]{ exec.spin(); });` để không chặn `main`
  — nhớ `join()`/dừng executor trước khi thoát.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Quên `join()`/`detach()` → `std::thread` huỷ khi còn joinable → `std::terminate`.
- ❌ Bộ đếm chia sẻ `++counter` không khoá → tổng sai (mất lượt tăng do race).
- ❌ `cv.wait(lk)` **không** kèm predicate → dính spurious wakeup / lost wakeup.
- ❌ `notify` khi đang giữ dữ liệu nhưng chưa cập nhật trạng thái mà predicate kiểm.
- ❌ Khoá hai mutex ở thứ tự khác nhau giữa các luồng → deadlock. Dùng `scoped_lock`.
- ❌ Truyền biến bằng tham chiếu vào thread mà không `std::ref` → thực ra là copy.
- ❌ `future` từ `std::async` không giữ lại → destructor của nó **chặn** (blocking) bất ngờ.

## E. Docs
- https://en.cppreference.com/w/cpp/thread/thread
- https://en.cppreference.com/w/cpp/thread/mutex
- https://en.cppreference.com/w/cpp/atomic/atomic
- https://en.cppreference.com/w/cpp/thread/condition_variable
- https://en.cppreference.com/w/cpp/thread/async
- ROS 2: https://docs.ros.org/en/humble/Concepts/Intermediate/About-Executors.html

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — N luồng cùng tăng một bộ đếm K lần; bảo vệ bằng
   `mutex`/`atomic` sao cho tổng = N*K một cách xác định (join hết rồi mới CHECK).
2. `exercises/ex2_advanced.cpp` — hàng đợi thông điệp thread-safe bằng `mutex` +
   `condition_variable` (producer/consumer) + demo `std::async`/`future`.

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
