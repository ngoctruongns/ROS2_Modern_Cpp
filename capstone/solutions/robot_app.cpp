// ============================================================================
// CAPSTONE — LỜI GIẢI / SOLUTION
// Hệ thống giám sát nhiệt độ robot, gồm 3 node chạy trên mini_ros:
//   SensorNode  : timer 200ms -> publish Temperature (tăng dần) lên "temperature"
//   MonitorNode : sub "temperature", nếu vượt "threshold" -> publish Alert + log
//   AlarmNode   : sub "alert", đếm số cảnh báo
// Executor chạy thời gian ảo 2s, bước 100ms -> hoàn toàn tất định.
//
// Tất cả kiến thức C++ modern được dùng ở đây — xem chú thích [NN].
// ============================================================================
#include "mini_ros/mini_ros.hpp"
#include "robot/messages.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

using namespace std::chrono_literals;   // [06] 200ms, 100ms, 2s

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// ------------------------------------------------------------------ SensorNode
class SensorNode : public mini_ros::Node {          // [10] kế thừa Node
public:
  explicit SensorNode(std::shared_ptr<mini_ros::Context> ctx)
      : Node("sensor", std::move(ctx)) {
    declare_parameter("base_celsius", 20.0);        // [08] tham số
    declare_parameter("step_celsius", 1.0);
    base_ = get_parameter<double>("base_celsius").value_or(20.0);   // [08] optional
    step_ = get_parameter<double>("step_celsius").value_or(1.0);

    pub_ = create_publisher<robot_msgs::Temperature>("temperature", mini_ros::QoS(10)); // [05]
    create_wall_timer(200ms, [this] { on_timer(); });               // [03][06]
  }
  int published() const { return count_; }

private:
  void on_timer() {
    robot_msgs::Temperature msg;
    msg.seq = static_cast<std::uint32_t>(count_);
    msg.celsius = base_ + step_ * count_;
    pub_->publish(std::move(msg));                   // [04] move message
    ++count_;
  }
  mini_ros::Publisher<robot_msgs::Temperature>::SharedPtr pub_;   // [01]
  double base_ = 20.0;
  double step_ = 1.0;
  int count_ = 0;
};

// ----------------------------------------------------------------- MonitorNode
class MonitorNode : public mini_ros::Node {
public:
  explicit MonitorNode(std::shared_ptr<mini_ros::Context> ctx)
      : Node("monitor", std::move(ctx)) {
    declare_parameter("threshold", 25.0);
    threshold_ = get_parameter<double>("threshold").value_or(25.0);

    alert_pub_ = create_publisher<robot_msgs::Alert>("alert", mini_ros::QoS(10));
    sub_ = create_subscription<robot_msgs::Temperature>(   // [05][03]
        "temperature", mini_ros::QoS(10),
        [this](std::shared_ptr<const robot_msgs::Temperature> m) { on_temp(m); });
  }
  int received() const { return received_; }
  int alerts() const { return alerts_; }
  double max_seen() const { return max_; }

private:
  void on_temp(const std::shared_ptr<const robot_msgs::Temperature>& m) {
    ++received_;
    max_ = std::max(max_, m->celsius);              // [07] <algorithm>
    if (m->celsius > threshold_) {
      ++alerts_;
      robot_msgs::Alert a;
      a.level = "WARN";
      a.message = "temperature over threshold";
      a.value = m->celsius;
      alert_pub_->publish(std::move(a));            // [04]
      log(mini_ros::LogLevel::Warn,                 // [11]
          "temp=" + std::to_string(m->celsius) + " > " + std::to_string(threshold_));
    }
  }
  mini_ros::Subscription<robot_msgs::Temperature>::SharedPtr sub_;
  mini_ros::Publisher<robot_msgs::Alert>::SharedPtr alert_pub_;
  double threshold_ = 25.0;
  int received_ = 0;
  int alerts_ = 0;
  double max_ = -1e9;
};

// ------------------------------------------------------------------- AlarmNode
class AlarmNode : public mini_ros::Node {
public:
  explicit AlarmNode(std::shared_ptr<mini_ros::Context> ctx)
      : Node("alarm", std::move(ctx)) {
    sub_ = create_subscription<robot_msgs::Alert>(
        "alert", mini_ros::QoS(10),
        [this](std::shared_ptr<const robot_msgs::Alert> a) {
          ++count_;
          last_value_ = a->value;
        });
  }
  int count() const { return count_; }
  double last_value() const { return last_value_; }

private:
  mini_ros::Subscription<robot_msgs::Alert>::SharedPtr sub_;
  int count_ = 0;
  double last_value_ = 0.0;
};

// ------------------------------------------------------------------------ main
int main() {
  auto ctx = mini_ros::init();                       // ~ rclcpp::init()

  auto sensor = std::make_shared<SensorNode>(ctx);   // [01] make_shared
  auto monitor = std::make_shared<MonitorNode>(ctx);
  auto alarm = std::make_shared<AlarmNode>(ctx);

  mini_ros::SingleThreadedExecutor exec(ctx);        // [10]
  exec.add_node(sensor);
  exec.add_node(monitor);
  exec.add_node(alarm);

  std::cout << "--- spin 2s (bước 100ms) ---\n";
  exec.spin_for(2s, 100ms);                          // [06] tất định
  std::cout << "--- xong ---\n\n";

  // Kỳ vọng (xem README để hiểu vì sao):
  // timer 200ms trong 2s -> 10 lần publish; nhiệt độ 20..29; threshold 25.
  // celsius > 25 -> {26,27,28,29} => 4 alert.
  CHECK(sensor->published() == 10);
  CHECK(monitor->received() == 10);
  CHECK(monitor->alerts() == 4);
  CHECK(alarm->count() == 4);
  CHECK(monitor->max_seen() == 29.0);
  CHECK(alarm->last_value() == 29.0);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉 — bạn đã ghép được cả hệ thống!\n"
                            : "\nCòn lỗi, xem [FAIL] ở trên.\n");
  return g_fail == 0 ? 0 : 1;
}
