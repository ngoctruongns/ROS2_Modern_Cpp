// ============================================================================
// Topic 10 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// Abstract base + pure virtual + override + virtual destructor + đa hình.
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

static int g_derived_dtor_calls = 0;

// ---- Interface trừu tượng ----------------------------------------------------
class SensorDriver {
public:
  explicit SensorDriver(std::string name) : name_(std::move(name)) {}

  virtual int read() = 0;                 // pure virtual -> abstract/interface

  const std::string& name() const { return name_; }

  // VIRTUAL destructor: bắt buộc cho base đa hình. Khi `delete base_ptr` (kể cả
  // qua unique_ptr<SensorDriver>), ~Lidar()/~Imu() sẽ chạy trước rồi tới ~SensorDriver().
  // Nếu thiếu `virtual`, phần dẫn xuất KHÔNG được huỷ -> UB / rò rỉ tài nguyên.
  virtual ~SensorDriver() = default;

protected:
  std::string name_;
};

// ---- Driver cụ thể 1 ---------------------------------------------------------
class Lidar : public SensorDriver {
public:
  Lidar() : SensorDriver("lidar") {}       // gọi constructor base
  int read() override { return 360; }       // override -> compiler kiểm tra chữ ký
  ~Lidar() override { ++g_derived_dtor_calls; }
};

// ---- Driver cụ thể 2 ---------------------------------------------------------
class Imu : public SensorDriver {
public:
  Imu() : SensorDriver("imu") {}
  int read() override { return 9; }
  ~Imu() override { ++g_derived_dtor_calls; }
};

int main() {
  std::vector<std::unique_ptr<SensorDriver>> drivers;
  drivers.push_back(std::make_unique<Lidar>());
  drivers.push_back(std::make_unique<Imu>());

  CHECK(drivers.size() == 2);
  CHECK(drivers[0]->name() == "lidar");
  CHECK(drivers[0]->read() == 360);
  CHECK(drivers[1]->name() == "imu");
  CHECK(drivers[1]->read() == 9);

  drivers.clear();
  CHECK(g_derived_dtor_calls == 2);         // virtual dtor -> ~Derived() chạy đúng

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
