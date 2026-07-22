# 07 — STL Containers & `<algorithm>`

> **Vì sao topic này quan trọng?** Code ROS 2 gần như luôn phải *chứa* dữ liệu (mẫu
> cảm biến, danh sách publisher, tham số) và *xử lý* dữ liệu (lọc, tính trung bình,
> tìm max, sắp xếp). STL cho bạn container sẵn sàng dùng và bộ `<algorithm>` viết một
> dòng thay cho vòng `for` thủ công dễ sai. Nắm vững chúng = code ngắn, đúng, nhanh.

---

## A. Basic

### 1. `std::vector<T>` — mảng động / dynamic array
Container dùng nhiều nhất. Phần tử nằm liên tục trong bộ nhớ (cache-friendly).

```cpp
std::vector<double> v;
v.push_back(1.5);            // copy/move một giá trị đã có
v.emplace_back(2.5);         // dựng phần tử TẠI CHỖ (in-place), tránh 1 lần copy/move
v.reserve(100);              // cấp trước sức chứa -> tránh cấp phát lại nhiều lần
v.size();                    // số phần tử hiện có
v.capacity();                // sức chứa đã cấp (>= size)
```
- `push_back(x)` cần một object rồi copy/move vào; `emplace_back(args...)` chuyển thẳng
  tham số cho constructor → hiệu quả hơn với kiểu phức tạp.
- `reserve(n)` khi biết trước số lượng: tránh realloc lặp lại (mỗi realloc = cấp mới +
  copy toàn bộ + huỷ cũ). **Lưu ý:** `reserve` đổi `capacity`, **không** đổi `size`.

### 2. `std::array<T, N>` — mảng tĩnh, kích thước cố định
Bọc mảng C thô, biết kích thước lúc biên dịch, không cấp phát động.

```cpp
std::array<int, 3> a{10, 20, 30};
a.size();        // 3 (hằng số biên dịch)
a[0]; a.at(1);
```

### 3. `std::pair<A, B>` — cặp hai giá trị
```cpp
std::pair<std::string, int> p{"imu", 5};
p.first;  p.second;
auto q = std::make_pair(1, 2.0);
```

### 4. `std::map` (ordered) vs `std::unordered_map` (hash)
| | `std::map` | `std::unordered_map` |
|---|---|---|
| Cấu trúc | cây cân bằng (red-black) | bảng băm (hash table) |
| Thứ tự khoá | **sắp xếp tăng dần** | **không có thứ tự** |
| Tra cứu/chèn | `O(log n)` | `O(1)` trung bình |
| Khi nào dùng | cần duyệt theo thứ tự khoá | chỉ cần tra cứu nhanh theo khoá |

→ Mặc định cần *lookup theo khoá cho nhanh* thì chọn `unordered_map`. Cần *duyệt có thứ
tự* (vd in ra theo alphabet) thì chọn `map`.

### 5. Duyệt bằng range-for + structured bindings (C++17)
```cpp
std::unordered_map<std::string, int> counts;
for (const auto& [key, value] : counts) {      // structured binding
  std::cout << key << " -> " << value << "\n";
}
```

### 6. `.at()` vs `operator[]` — CẨN THẬN với `[]`
```cpp
std::map<std::string, int> m;
m["a"] = 1;
m.at("a");        // trả ref; NÉM std::out_of_range nếu không có khoá -> an toàn để đọc
m["b"];           // TÁC DỤNG PHỤ: nếu "b" chưa có, TỰ TẠO "b"=0 rồi trả về!
```
> `operator[]` trên map/unordered_map **chèn phần tử mặc định** nếu khoá chưa tồn tại.
> Muốn chỉ đọc (không vô tình chèn) → dùng `.at()` hoặc `.find()`.
> Trên `vector`, `[]` không kiểm tra biên còn `.at()` kiểm tra biên và ném khi vượt.

### 7. `.find()` vs `.count()` (và ghi chú `.contains()`)
```cpp
auto it = m.find("a");
if (it != m.end()) { /* có: dùng it->second */ }
bool has = m.count("a") > 0;      // count: 0 hoặc 1 với map/unordered_map
```
> `.contains()` (trả `bool`) chỉ có từ **C++20**. Trong repo C++17 này ta dùng
> `find() != end()` (còn cho luôn iterator để dùng tiếp) hoặc `count()`.

---

## B. Nâng cao / Advanced — `<algorithm>` & `<numeric>`

Hầu hết thuật toán nhận cặp iterator `[begin, end)`. `begin()` trỏ phần tử đầu,
`end()` trỏ **sau** phần tử cuối (vị trí "quá cuối").

### 8. `std::sort` (+ lambda comparator)
```cpp
std::sort(v.begin(), v.end());                       // tăng dần mặc định
std::sort(v.begin(), v.end(),
          [](double a, double b){ return a > b; });  // giảm dần bằng lambda
```

### 9. `std::find_if` — tìm phần tử đầu thoả điều kiện
```cpp
auto it = std::find_if(v.begin(), v.end(),
                       [](double x){ return x > 10.0; });
if (it != v.end()) { /* thấy tại *it */ }
```

### 10. `std::count_if` — đếm phần tử thoả điều kiện
```cpp
long n = std::count_if(v.begin(), v.end(),
                       [](double x){ return x < 0.0; });
```

### 11. `std::max_element` / `std::min_element` — trả **iterator**
```cpp
auto it = std::max_element(v.begin(), v.end());
double maxv = (it != v.end()) ? *it : 0.0;   // nhớ deref, và kiểm tra rỗng
```

### 12. `std::accumulate` (trong `<numeric>`) — tổng / gộp
```cpp
#include <numeric>
double sum  = std::accumulate(v.begin(), v.end(), 0.0);   // hạt giống 0.0 -> double!
double mean = v.empty() ? 0.0 : sum / v.size();
```
> Cẩn thận kiểu của giá trị khởi đầu: `accumulate(..., 0)` (int) sẽ **làm tròn** khi
> cộng `double`. Dùng `0.0` cho tổng số thực.

### 13. `std::transform` — ánh xạ từng phần tử
```cpp
std::transform(v.begin(), v.end(), v.begin(),
               [](double x){ return x * 2.0; });          // scale tại chỗ
```

### 14. Erase–remove idiom — xoá phần tử theo điều kiện khỏi vector
```cpp
v.erase(std::remove_if(v.begin(), v.end(),
                       [](double x){ return x < 0.0; }),
        v.end());
```
> `std::remove_if` **không** xoá thật: nó dồn các phần tử giữ lại lên đầu và trả về
> iterator "đuôi rác". Phải gọi `vector::erase(...)` để cắt phần đuôi đó đi. Quên bước
> `erase` là lỗi kinh điển.

> **Ghi chú C++20 (không dùng trong code repo này):** ranges giúp viết gọn hơn nhiều,
> vd `std::ranges::sort(v)`, `v | std::views::filter(...)`, hay `std::erase_if(v, pred)`.
> Ở đây ta bám C++17 nên vẫn dùng cặp iterator như trên.

---

## C. Liên hệ ROS 2 / Mapping to rclcpp

```cpp
// 1) Buffer mẫu cảm biến trong vector (vd gom nhiều lần đo trước khi xử lý):
std::vector<double> ranges;
ranges.reserve(360);                 // biết trước số tia laser -> reserve
for (float r : scan_msg->ranges) ranges.push_back(r);

// 2) Bảng ánh xạ topic -> publisher, tra cứu nhanh theo tên:
std::unordered_map<std::string, rclcpp::Publisher<Msg>::SharedPtr> pubs_;
auto it = pubs_.find(topic);
if (it != pubs_.end()) it->second->publish(msg);

// 3) Xử lý mảng dữ liệu range/laser bằng <algorithm>:
auto closest = std::min_element(ranges.begin(), ranges.end());  // vật gần nhất
double mean  = std::accumulate(ranges.begin(), ranges.end(), 0.0) / ranges.size();
```
- `sensor_msgs::msg::LaserScan::ranges` bản chất là `std::vector<float>`.
- Lọc giá trị `inf`/ngoài tầm bằng erase–remove trước khi tính toán thống kê.

---

## D. Lỗi thường gặp / Common pitfalls
- ❌ Dùng `map[key]` để *kiểm tra* khoá → vô tình **chèn** phần tử mặc định. Dùng `find`.
- ❌ Quên `vector::erase` sau `std::remove_if` → phần tử "bị xoá" vẫn còn ở đuôi.
- ❌ `std::accumulate(v.begin(), v.end(), 0)` với vector double → tổng bị **làm tròn** về int.
- ❌ Deref `max_element`/`find_if` mà không kiểm tra `!= end()` (đặc biệt khi vector rỗng).
- ❌ Nhầm `size()` với `capacity()`; `reserve` không tạo phần tử (size vẫn 0).
- ❌ Dùng `.contains()` (C++20) trong code C++17 → không biên dịch.
- ❌ Lưu iterator/con trỏ vào phần tử vector rồi `push_back` làm realloc → iterator hỏng (dangling).

## E. Docs
- https://en.cppreference.com/w/cpp/container/vector
- https://en.cppreference.com/w/cpp/container/unordered_map
- https://en.cppreference.com/w/cpp/container/map
- https://en.cppreference.com/w/cpp/algorithm
- https://en.cppreference.com/w/cpp/algorithm/accumulate
- Effective STL — Scott Meyers.

---

## Bài tập / Exercises
1. `exercises/ex1_basic.cpp` — dựng & thao tác `std::vector<double>` và
   `std::unordered_map<std::string,int>` (đếm), duyệt bằng structured bindings,
   dùng `.at()` / `find()`.
2. `exercises/ex2_advanced.cpp` — pipeline dữ liệu cảm biến trên `std::vector<double>`:
   lọc giá trị ngoài tầm (erase–remove), tính trung bình (`accumulate`), tìm max
   (`max_element`), sắp xếp (`sort`), và scale (`transform`).

Build & chạy:
```bash
cmake -B build && cmake --build build
./build/ex1_basic        # đề của bạn (sẽ FAIL cho tới khi bạn điền TODO)
./build/sol_ex1_basic    # lời giải
```
