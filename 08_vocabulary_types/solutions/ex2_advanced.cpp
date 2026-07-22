// ============================================================================
// Topic 08 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// std::variant + std::visit + overloaded visitor + std::monostate
// ============================================================================
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <variant>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

using ParamValue = std::variant<bool, int64_t, double, std::string>;

template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

// visit gọi đúng nhánh theo kiểu variant đang giữ.
std::string stringify(const ParamValue& v) {
  return std::visit(overload{
      [](bool b)               { return std::string("bool:") + (b ? "true" : "false"); },
      [](int64_t i)            { return "int:" + std::to_string(i); },
      [](double d)             { return "double:" + std::to_string(d); },
      [](const std::string& s) { return "str:" + s; },
  }, v);
}

int main() {
  std::map<std::string, ParamValue> params;
  params["use_sim_time"] = false;
  params["baud"]         = int64_t{115200};
  params["rate_hz"]      = 20.0;
  params["frame_id"]     = std::string("base");

  CHECK(std::holds_alternative<bool>(params["use_sim_time"]));
  CHECK(std::holds_alternative<int64_t>(params["baud"]));
  CHECK(std::holds_alternative<double>(params["rate_hz"]));
  CHECK(std::holds_alternative<std::string>(params["frame_id"]));

  CHECK(params["use_sim_time"].index() == 0);
  CHECK(params["frame_id"].index() == 3);

  auto* pb = std::get_if<int64_t>(&params["baud"]);
  CHECK(pb != nullptr && *pb == 115200);
  CHECK(std::get_if<double>(&params["baud"]) == nullptr);

  CHECK(stringify(params["use_sim_time"]) == "bool:false");
  CHECK(stringify(params["baud"])         == "int:115200");
  CHECK(stringify(params["frame_id"])     == "str:base");

  std::variant<std::monostate, int> maybe;
  CHECK(maybe.index() == 0);
  CHECK(std::holds_alternative<std::monostate>(maybe));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
