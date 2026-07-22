// ============================================================================
// "Message definitions" tí hon — mô phỏng các file .msg của ROS 2.
// Trong ROS thật đây là std_msgs/sensor_msgs được sinh code tự động.
// ============================================================================
#pragma once
#include <cstdint>
#include <string>

namespace robot_msgs {

// ~ sensor_msgs/Temperature (rút gọn)
struct Temperature {
  double celsius = 0.0;
  std::uint32_t seq = 0;   // số thứ tự để kiểm tra thứ tự nhận
};

// ~ một message cảnh báo tự định nghĩa
struct Alert {
  std::string level;       // "WARN" / "ERROR"
  std::string message;
  double value = 0.0;
};

}  // namespace robot_msgs
