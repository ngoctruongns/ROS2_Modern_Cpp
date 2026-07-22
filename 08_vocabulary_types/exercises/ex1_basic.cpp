// ============================================================================
// Topic 08 — Vocabulary Types — Bài 1 (BASIC)
// Mục tiêu: std::optional (has_value/value/value_or/*, std::nullopt) +
//           std::string_view (nhìn chuỗi không sao chép, tránh dangling).
//
// Cách làm: điền code vào các chỗ có "TODO". Chạy lại tới khi tất cả in [PASS].
//   cmake -B build && cmake --build build && ./build/ex1_basic
// Đối chiếu lời giải: ./build/sol_ex1_basic
// ============================================================================
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>

// --- test harness nhỏ ---
static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// "Bảng cấu hình" giả lập: tên tham số -> giá trị int.
static const std::map<std::string, int> g_config = {
    {"baud", 115200},
    {"port", 8080},
    {"retries", 3},
};

// TODO 1: Tra cứu `key` trong g_config.
//   - Nếu tìm thấy: trả về std::optional<int> chứa giá trị.
//   - Nếu KHÔNG thấy: trả về std::nullopt.
std::optional<int> lookup(const std::string& key) {
  (void)key;
  return std::nullopt; // TODO: dùng g_config.find(...) và trả optional
}

// TODO 2: Kiểm tra một key có tiền tố "ros__" hay không, KHÔNG tạo std::string mới.
//   Dùng std::string_view (gợi ý: sv.substr / sv.compare / sv.rfind(prefix, 0)).
bool has_ros_prefix(std::string_view key) {
  (void)key;
  return false; // TODO
}

int main() {
  // --- optional: tìm thấy ---
  auto b = lookup("baud");
  CHECK(b.has_value());
  CHECK(b && *b == 115200);          // dùng * để lấy giá trị khi đã chắc có
  CHECK(b && b.value() == 115200);   // .value() cũng lấy giá trị (có guard tránh ném)

  // --- optional: không tìm thấy ---
  auto miss = lookup("does_not_exist");
  CHECK(!miss.has_value());
  CHECK(!miss);                      // optional rỗng ép sang false

  // --- value_or: giá trị mặc định khi rỗng ---
  int retries = lookup("retries").value_or(1);
  CHECK(retries == 3);
  int timeout = lookup("timeout").value_or(30);   // "timeout" không có -> mặc định
  CHECK(timeout == 30);

  // --- string_view: soi key mà không sao chép ---
  CHECK(has_ros_prefix("ros__log_level"));
  CHECK(!has_ros_prefix("baud"));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
