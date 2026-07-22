// ============================================================================
// Topic 10 — Class Design — Bài 2 (ADVANCED)
// Mục tiêu: abstract base class / interface, pure virtual, override, virtual
//           destructor, gọi constructor base, lưu & gọi đa hình qua base pointer.
//
// Bối cảnh (ROS 2 flavored): interface `SensorDriver` với `read()` thuần ảo.
//   Hai driver cụ thể (Lidar, Imu) cài đặt khác nhau. Ta cất chúng vào
//   std::vector<std::unique_ptr<SensorDriver>> và gọi read() qua con trỏ base
//   -> dynamic dispatch chọn đúng phiên bản của từng lớp.
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

// Đếm số lần destructor CỦA LỚP DẪN XUẤT chạy — để chứng minh virtual dtor hoạt động.
static int g_derived_dtor_calls = 0;

// ---- Interface trừu tượng ----------------------------------------------------
class SensorDriver {
public:
  explicit SensorDriver(std::string name) : name_(std::move(name)) {}

  // TODO 1: biến read() thành PURE VIRTUAL (= 0) để SensorDriver là abstract.
  //   (Hiện đang có thân hàm giả để file biên dịch; hãy đổi thành pure virtual.)
  virtual int read() { return -1; }

  const std::string& name() const { return name_; }

  // TODO 2: destructor phải là VIRTUAL để `delete base_ptr` huỷ đúng phần dẫn xuất.
  //   (Hiện KHÔNG virtual -> nếu để vậy, ~Lidar()/~Imu() sẽ không chạy khi
  //    huỷ qua unique_ptr<SensorDriver> -> CHECK đếm dtor sẽ FAIL.)
  ~SensorDriver() = default;

protected:
  std::string name_;
};

// ---- Driver cụ thể 1 ---------------------------------------------------------
class Lidar : public SensorDriver {
public:
  // TODO 3: gọi constructor base SensorDriver("lidar") ở init-list.
  Lidar() : SensorDriver("lidar") {}

  // TODO 4: override read() trả về 360 (số điểm quét), dùng từ khoá `override`.
  //   (Hiện chưa override -> sẽ chạy read() mặc định của base = -1.)

  ~Lidar() { ++g_derived_dtor_calls; }
};

// ---- Driver cụ thể 2 ---------------------------------------------------------
class Imu : public SensorDriver {
public:
  // TODO 5: gọi constructor base SensorDriver("imu").
  Imu() : SensorDriver("imu") {}

  // TODO 6: override read() trả về 9 (số trục dữ liệu), dùng `override`.

  ~Imu() { ++g_derived_dtor_calls; }
};

int main() {
  std::vector<std::unique_ptr<SensorDriver>> drivers;
  drivers.push_back(std::make_unique<Lidar>());
  drivers.push_back(std::make_unique<Imu>());

  // Gọi đa hình qua con trỏ base -> đúng phiên bản của từng lớp.
  CHECK(drivers.size() == 2);
  CHECK(drivers[0]->name() == "lidar");
  CHECK(drivers[0]->read() == 360);   // Lidar::read()
  CHECK(drivers[1]->name() == "imu");
  CHECK(drivers[1]->read() == 9);     // Imu::read()

  // Huỷ tất cả: với virtual dtor, ~Lidar() và ~Imu() PHẢI chạy (2 lần).
  drivers.clear();
  CHECK(g_derived_dtor_calls == 2);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
