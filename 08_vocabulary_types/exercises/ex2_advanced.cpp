// ============================================================================
// Topic 08 — Vocabulary Types — Bài 2 (ADVANCED)
// Mục tiêu: std::variant (get/get_if/holds_alternative/index/visit) +
//           overloaded visitor, std::monostate; nhắc tới std::any.
// Bối cảnh: dựng ParamValue giống rclcpp::ParameterValue — một variant trên
//           bool/int64_t/double/string — rồi "stringify" từng giá trị.
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
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

// Giống rclcpp::ParameterValue: giữ đúng MỘT trong các kiểu này.
using ParamValue = std::variant<bool, int64_t, double, std::string>;

// Mẫu "overloaded visitor" cổ điển — hợp lệ trong C++17.
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

// TODO 1: "stringify" một ParamValue thành chuỗi có tiền tố kiểu:
//   bool   -> "bool:true" / "bool:false"
//   int64  -> "int:<n>"           (dùng std::to_string)
//   double -> "double:<x>"        (dùng std::to_string)
//   string -> "str:<s>"
// Gợi ý: std::visit với overload{...}, HOẶC chuỗi if(get_if<...>).
std::string stringify(const ParamValue& v) {
  (void)v;
  return "TODO"; // TODO: thay bằng std::visit(overload{...}, v)
}

int main() {
  std::map<std::string, ParamValue> params;
  params["use_sim_time"] = false;                 // bool
  params["baud"]         = int64_t{115200};       // int64_t
  params["rate_hz"]      = 20.0;                   // double
  params["frame_id"]     = std::string("base");   // string

  // --- truy vấn kiểu (giống Parameter::get_type) ---
  CHECK(std::holds_alternative<bool>(params["use_sim_time"]));
  CHECK(std::holds_alternative<int64_t>(params["baud"]));
  CHECK(std::holds_alternative<double>(params["rate_hz"]));
  CHECK(std::holds_alternative<std::string>(params["frame_id"]));

  // --- index() cho biết nhánh nào đang giữ (bool=0, int64=1, double=2, string=3) ---
  CHECK(params["use_sim_time"].index() == 0);
  CHECK(params["frame_id"].index() == 3);

  // --- get_if: lấy an toàn (trả con trỏ hoặc nullptr) ---
  auto* pb = std::get_if<int64_t>(&params["baud"]);
  CHECK(pb != nullptr && *pb == 115200);
  CHECK(std::get_if<double>(&params["baud"]) == nullptr); // sai kiểu -> nullptr

  // --- stringify qua visit ---
  CHECK(stringify(params["use_sim_time"]) == "bool:false");
  CHECK(stringify(params["baud"])         == "int:115200");
  CHECK(stringify(params["frame_id"])     == "str:base");

  // --- monostate: variant có trạng thái "chưa set" ---
  std::variant<std::monostate, int> maybe;         // mặc định = monostate
  CHECK(maybe.index() == 0);
  CHECK(std::holds_alternative<std::monostate>(maybe));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
