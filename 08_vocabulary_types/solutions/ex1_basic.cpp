// ============================================================================
// Topic 08 — Bài 1 (BASIC) — LỜI GIẢI / SOLUTION
// std::optional + std::string_view
// ============================================================================
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

static const std::map<std::string, int> g_config = {
    {"baud", 115200},
    {"port", 8080},
    {"retries", 3},
};

// Tìm thấy -> optional chứa giá trị; không thấy -> std::nullopt.
std::optional<int> lookup(const std::string& key) {
  auto it = g_config.find(key);
  if (it == g_config.end()) return std::nullopt;
  return it->second;
}

// So khớp tiền tố trên string_view: 0 cấp phát, 0 sao chép.
bool has_ros_prefix(std::string_view key) {
  constexpr std::string_view prefix = "ros__";
  return key.substr(0, prefix.size()) == prefix;
}

int main() {
  auto b = lookup("baud");
  CHECK(b.has_value());
  CHECK(b && *b == 115200);
  CHECK(b && b.value() == 115200);

  auto miss = lookup("does_not_exist");
  CHECK(!miss.has_value());
  CHECK(!miss);

  int retries = lookup("retries").value_or(1);
  CHECK(retries == 3);
  int timeout = lookup("timeout").value_or(30);
  CHECK(timeout == 30);

  CHECK(has_ros_prefix("ros__log_level"));
  CHECK(!has_ros_prefix("baud"));

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
