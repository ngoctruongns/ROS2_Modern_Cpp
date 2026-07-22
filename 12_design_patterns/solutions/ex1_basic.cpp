// ============================================================================
// Topic 12 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// Strategy + Factory.
// ============================================================================
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

struct VelocityController {
  virtual double compute(double error) = 0;
  virtual std::string name() const = 0;
  virtual ~VelocityController() = default;
};

struct ConstantController : VelocityController {
  double v;
  explicit ConstantController(double v_) : v(v_) {}
  double compute(double error) override {
    (void)error;
    return v;                                   // hằng số, kệ error
  }
  std::string name() const override { return "constant"; }
};

struct ProportionalController : VelocityController {
  double kp;
  explicit ProportionalController(double kp_) : kp(kp_) {}
  double compute(double error) override {
    return kp * error;                          // điều khiển tỉ lệ
  }
  std::string name() const override { return "proportional"; }
};

class ControllerFactory {
public:
  using Creator = std::function<std::unique_ptr<VelocityController>()>;

  void register_type(const std::string& key, Creator c) {
    creators_[key] = std::move(c);
  }

  std::unique_ptr<VelocityController> create(const std::string& key) const {
    auto it = creators_.find(key);
    if (it == creators_.end()) return nullptr;  // tên lạ -> nullptr, không crash
    return it->second();                        // gọi creator -> object mới
  }

private:
  std::map<std::string, Creator> creators_;
};

int main() {
  ControllerFactory factory;
  factory.register_type("constant",
                        [] { return std::make_unique<ConstantController>(0.5); });
  factory.register_type("proportional",
                        [] { return std::make_unique<ProportionalController>(2.0); });

  auto c1 = factory.create("constant");
  CHECK(c1 != nullptr);
  CHECK(c1 && c1->name() == "constant");

  auto c2 = factory.create("proportional");
  CHECK(c2 != nullptr);
  CHECK(c2 && c2->name() == "proportional");

  auto bad = factory.create("does_not_exist");
  CHECK(bad == nullptr);

  const double err = 3.0;
  CHECK(c1 && std::abs(c1->compute(err) - 0.5) < 1e-9);
  CHECK(c2 && std::abs(c2->compute(err) - 6.0) < 1e-9);

  std::unique_ptr<VelocityController> active = factory.create("constant");
  double before = active ? active->compute(err) : -999.0;
  active = factory.create("proportional");
  double after = active ? active->compute(err) : -999.0;
  CHECK(std::abs(before - 0.5) < 1e-9);
  CHECK(std::abs(after - 6.0) < 1e-9);
  CHECK(std::abs(before - after) > 1e-9);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
