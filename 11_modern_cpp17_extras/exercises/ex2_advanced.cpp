// ============================================================================
// Topic 11 — Modern C++17 Extras — Bài 2 (ADVANCED)
// Mục tiêu: constexpr cấu hình lúc biên dịch, inline constexpr (kiểu header),
//           nested namespace C++17, và state machine dùng enum class.
// Bối cảnh: mô phỏng cấu hình + máy trạng thái của một node ROS 2 (drive base).
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <string>
#include <filesystem>   // C++17; thao tác path cơ bản không cần cờ link trên g++9

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// --- Nested namespace C++17: gom hằng cấu hình của node ---
namespace robot::config {
  // TODO 1: khai báo hằng inline constexpr cho độ sâu QoS mặc định = 10.
  //         (inline để có thể đặt trong header dùng nhiều nơi mà không lỗi
  //          "multiple definition".)
  // inline constexpr int kQosDepth = 10;
  inline constexpr int kQosDepth = 0;               // TODO: sửa thành 10
  inline constexpr const char* kCmdVelTopic = "/cmd_vel";
}  // namespace robot::config

namespace cfg = robot::config;   // namespace alias cho gọn

// --- Anonymous namespace: internal linkage (chỉ thấy trong file này) ---
namespace {
  // TODO 2: constexpr tính số byte của một hàng đợi tin cmd_vel:
  //         mỗi phần tử coi như 8 byte -> depth * 8.
  constexpr int queue_bytes(int depth) {
    (void)depth;
    return 0;  // TODO: return depth * 8;
  }
}

// TODO 3: bỏ comment các static_assert sau KHI đã cài kQosDepth và queue_bytes.
// static_assert(cfg::kQosDepth == 10, "QoS depth mặc định phải là 10");
// static_assert(queue_bytes(cfg::kQosDepth) == 80, "queue_bytes sai");

// --- State machine bằng enum class ---
enum class DriveState { Stopped, Driving, Faulted };
enum class Command    { Start, Stop, Fault, Reset };

// TODO 4: cài hàm chuyển trạng thái constexpr:
//   Stopped + Start  -> Driving
//   Driving + Stop   -> Stopped
//   (bất kỳ) + Fault -> Faulted
//   Faulted + Reset  -> Stopped
//   còn lại          -> giữ nguyên state cũ
constexpr DriveState next(DriveState s, Command c) {
  (void)c;
  return s;  // TODO: cài đặt bảng chuyển ở trên
}

int main() {
  // --- constexpr / inline constexpr config ---
  CHECK(cfg::kQosDepth == 10);
  CHECK(std::string(cfg::kCmdVelTopic) == "/cmd_vel");
  CHECK(queue_bytes(cfg::kQosDepth) == 80);

  // --- state machine ---
  DriveState s = DriveState::Stopped;
  s = next(s, Command::Start);
  CHECK(s == DriveState::Driving);
  s = next(s, Command::Stop);
  CHECK(s == DriveState::Stopped);
  s = next(DriveState::Driving, Command::Fault);
  CHECK(s == DriveState::Faulted);
  s = next(s, Command::Reset);
  CHECK(s == DriveState::Stopped);
  // lệnh không hợp lệ -> giữ nguyên
  CHECK(next(DriveState::Stopped, Command::Stop) == DriveState::Stopped);

  // --- std::filesystem: chỉ thao tác trên path (không đụng đĩa) ---
  std::filesystem::path p = "/opt/ros/params.yaml";
  CHECK(p.extension() == ".yaml");
  CHECK(p.filename() == "params.yaml");

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
