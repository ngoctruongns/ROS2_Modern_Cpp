// ============================================================================
// Topic 12 — Design Patterns — Bài 1 (BASIC)
// Mục tiêu: STRATEGY + FACTORY.
//   - Strategy: một interface VelocityController, nhiều thuật toán cắm-rút được.
//   - Factory : tạo controller theo TÊN chuỗi (mô phỏng pluginlib: đăng ký/tra cứu
//               trong std::map<string, hàm-tạo>).
//
// Cách làm: điền code vào các chỗ "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// ---- STRATEGY: interface chung cho mọi bộ điều khiển vận tốc ----
// Đa hình -> BẮT BUỘC có virtual destructor (xem topic 10).
struct VelocityController {
  virtual double compute(double error) = 0;   // tính vận tốc lệnh từ sai số
  virtual std::string name() const = 0;
  virtual ~VelocityController() = default;
};

// TODO 1: ConstantController — luôn trả về một hằng số `v` bất kể error.
//         name() trả "constant".
struct ConstantController : VelocityController {
  double v;
  explicit ConstantController(double v_) : v(v_) {}
  double compute(double error) override {
    (void)error;
    return 0.0;                 // TODO: trả về v
  }
  std::string name() const override { return "constant"; }  // (đã cho sẵn)
};

// TODO 2: ProportionalController — điều khiển tỉ lệ: compute(error) = kp * error.
//         name() trả "proportional".
struct ProportionalController : VelocityController {
  double kp;
  explicit ProportionalController(double kp_) : kp(kp_) {}
  double compute(double error) override {
    (void)error;
    return 0.0;                 // TODO: trả về kp * error
  }
  std::string name() const override { return "proportional"; }  // (đã cho sẵn)
};

// ---- FACTORY: tạo controller theo tên (mô phỏng pluginlib) ----
class ControllerFactory {
public:
  using Creator = std::function<std::unique_ptr<VelocityController>()>;

  // TODO 3: đăng ký một creator dưới một tên. (Lưu vào map creators_.)
  void register_type(const std::string& key, Creator c) {
    (void)key; (void)c;
    // TODO
  }

  // TODO 4: tra cứu theo tên và gọi creator để tạo object mới.
  //         Nếu tên KHÔNG tồn tại -> trả nullptr (không crash!).
  std::unique_ptr<VelocityController> create(const std::string& key) const {
    (void)key;
    return nullptr;             // TODO
  }

private:
  std::map<std::string, Creator> creators_;
};

int main() {
  ControllerFactory factory;
  // Đăng ký hai chiến lược. "constant" luôn phát 0.5; "proportional" có kp = 2.0.
  factory.register_type("constant",
                        [] { return std::make_unique<ConstantController>(0.5); });
  factory.register_type("proportional",
                        [] { return std::make_unique<ProportionalController>(2.0); });

  // --- Factory tạo đúng kiểu theo tên ---
  auto c1 = factory.create("constant");
  CHECK(c1 != nullptr);
  CHECK(c1 && c1->name() == "constant");

  auto c2 = factory.create("proportional");
  CHECK(c2 != nullptr);
  CHECK(c2 && c2->name() == "proportional");

  // --- Tên lạ -> nullptr, KHÔNG crash ---
  auto bad = factory.create("does_not_exist");
  CHECK(bad == nullptr);

  // --- Hành vi Strategy: mỗi controller tính khác nhau cho cùng error ---
  const double err = 3.0;
  CHECK(c1 && std::abs(c1->compute(err) - 0.5) < 1e-9);        // hằng số, kệ error
  CHECK(c2 && std::abs(c2->compute(err) - 6.0) < 1e-9);        // kp*err = 2*3

  // --- Đổi chiến lược lúc runtime làm đổi output ---
  std::unique_ptr<VelocityController> active = factory.create("constant");
  double before = active ? active->compute(err) : -999.0;
  active = factory.create("proportional");   // cắm chiến lược khác
  double after = active ? active->compute(err) : -999.0;
  CHECK(std::abs(before - 0.5) < 1e-9);
  CHECK(std::abs(after - 6.0) < 1e-9);
  CHECK(std::abs(before - after) > 1e-9);    // swap thực sự đổi hành vi

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
