// ============================================================================
// Topic 11 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
// constexpr config, inline constexpr, nested namespace C++17, state machine
// bằng enum class, và std::filesystem (path-only).
// ============================================================================
#include <iostream>
#include <string>
#include <filesystem>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// Nested namespace C++17 + inline constexpr (an toàn khi đặt trong header).
namespace robot::config {
  inline constexpr int         kQosDepth     = 10;
  inline constexpr const char* kCmdVelTopic  = "/cmd_vel";
}  // namespace robot::config

namespace cfg = robot::config;   // alias

// Anonymous namespace -> internal linkage.
namespace {
  constexpr int queue_bytes(int depth) { return depth * 8; }
}

// Kiểm tra cấu hình ngay lúc biên dịch.
static_assert(cfg::kQosDepth == 10, "QoS depth mặc định phải là 10");
static_assert(queue_bytes(cfg::kQosDepth) == 80, "queue_bytes sai");

enum class DriveState { Stopped, Driving, Faulted };
enum class Command    { Start, Stop, Fault, Reset };

// Bảng chuyển trạng thái, viết dạng constexpr -> dùng được trong static_assert.
constexpr DriveState next(DriveState s, Command c) {
  if (c == Command::Fault) return DriveState::Faulted;
  switch (s) {
    case DriveState::Stopped: return c == Command::Start ? DriveState::Driving : s;
    case DriveState::Driving: return c == Command::Stop  ? DriveState::Stopped : s;
    case DriveState::Faulted: return c == Command::Reset ? DriveState::Stopped : s;
  }
  return s;
}

// Vì next() là constexpr, có thể kiểm tra chuyển trạng thái lúc biên dịch:
static_assert(next(DriveState::Stopped, Command::Start) == DriveState::Driving);
static_assert(next(DriveState::Driving, Command::Fault) == DriveState::Faulted);
static_assert(next(DriveState::Faulted, Command::Reset) == DriveState::Stopped);

int main() {
  CHECK(cfg::kQosDepth == 10);
  CHECK(std::string(cfg::kCmdVelTopic) == "/cmd_vel");
  CHECK(queue_bytes(cfg::kQosDepth) == 80);

  DriveState s = DriveState::Stopped;
  s = next(s, Command::Start);
  CHECK(s == DriveState::Driving);
  s = next(s, Command::Stop);
  CHECK(s == DriveState::Stopped);
  s = next(DriveState::Driving, Command::Fault);
  CHECK(s == DriveState::Faulted);
  s = next(s, Command::Reset);
  CHECK(s == DriveState::Stopped);
  CHECK(next(DriveState::Stopped, Command::Stop) == DriveState::Stopped);

  std::filesystem::path p = "/opt/ros/params.yaml";
  CHECK(p.extension() == ".yaml");
  CHECK(p.filename() == "params.yaml");

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
