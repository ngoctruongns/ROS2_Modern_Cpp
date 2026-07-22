// ============================================================================
// CAPSTONE — ĐỀ BÀI / EXERCISE
// Nhiệm vụ: dùng framework mini_ros (trong include/mini_ros/mini_ros.hpp — ĐỌC nó
// như tài liệu rclcpp thu nhỏ) để xây hệ thống giám sát nhiệt độ 3 node.
//
// Bạn CHỈ cần điền các chỗ "TODO". main() và các CHECK đã viết sẵn.
// File này compile được ngay, nhưng các CHECK sẽ FAIL cho tới khi bạn hoàn thiện.
//
//   cmake -B build && cmake --build build
//   ./build/robot_app          # đề của bạn
//   ./build/sol_robot_app      # lời giải để đối chiếu
//
// Yêu cầu hành vi (tất định):
//   SensorNode : mỗi 200ms publish 1 Temperature lên "temperature".
//                celsius = base_celsius + step_celsius * count (count bắt đầu 0).
//   MonitorNode: sub "temperature"; đếm số nhận; lưu max; nếu celsius > threshold
//                thì tăng bộ đếm alert, publish Alert lên "alert", và log WARN.
//   AlarmNode  : sub "alert"; đếm số cảnh báo; lưu value cảnh báo cuối.
// Với spin_for(2s, 100ms): 10 publish, nhiệt 20..29, threshold 25 -> 4 alert.
//
// GỢI Ý API (xem chi tiết trong mini_ros.hpp):
//   create_publisher<MsgT>("topic", mini_ros::QoS(10))            -> Publisher::SharedPtr
//   create_subscription<MsgT>("topic", QoS, callback)             -> Subscription::SharedPtr
//   create_wall_timer(200ms, [this]{ ... });
//   declare_parameter("name", value);  get_parameter<T>("name").value_or(default);
//   pub->publish(std::move(msg));       log(mini_ros::LogLevel::Warn, "...");
// ============================================================================
#include "mini_ros/mini_ros.hpp"
#include "robot/messages.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

using namespace std::chrono_literals;

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

// ------------------------------------------------------------------ SensorNode
class SensorNode : public mini_ros::Node {
public:
  explicit SensorNode(std::shared_ptr<mini_ros::Context> ctx)
      : Node("sensor", std::move(ctx)) {
    // TODO 1: declare_parameter "base_celsius"=20.0, "step_celsius"=1.0;
    //         đọc chúng ra base_/step_ bằng get_parameter<double>(...).value_or(...).
    // TODO 2: tạo pub_ = create_publisher<robot_msgs::Temperature>("temperature", QoS(10));
    // TODO 3: create_wall_timer(200ms, [this]{ on_timer(); });
  }
  int published() const { return count_; }

private:
  void on_timer() {
    // TODO 4: tạo Temperature{ celsius = base_ + step_*count_, seq = count_ },
    //         publish bằng std::move, rồi ++count_.
  }
  mini_ros::Publisher<robot_msgs::Temperature>::SharedPtr pub_;
  double base_ = 20.0;
  double step_ = 1.0;
  int count_ = 0;
};

// ----------------------------------------------------------------- MonitorNode
class MonitorNode : public mini_ros::Node {
public:
  explicit MonitorNode(std::shared_ptr<mini_ros::Context> ctx)
      : Node("monitor", std::move(ctx)) {
    // TODO 5: declare + đọc "threshold"=25.0 vào threshold_.
    // TODO 6: alert_pub_ = create_publisher<robot_msgs::Alert>("alert", QoS(10));
    // TODO 7: sub_ = create_subscription<robot_msgs::Temperature>(
    //             "temperature", QoS(10),
    //             [this](std::shared_ptr<const robot_msgs::Temperature> m){ on_temp(m); });
  }
  int received() const { return received_; }
  int alerts() const { return alerts_; }
  double max_seen() const { return max_; }

private:
  void on_temp(const std::shared_ptr<const robot_msgs::Temperature>& m) {
    (void)m;
    // TODO 8: ++received_; cập nhật max_ (std::max); nếu m->celsius > threshold_ thì
    //         ++alerts_, publish Alert (level="WARN", value=m->celsius) và log WARN.
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
    // TODO 9: sub_ = create_subscription<robot_msgs::Alert>("alert", QoS(10),
    //             [this](std::shared_ptr<const robot_msgs::Alert> a){
    //                ++count_; last_value_ = a->value; });
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
  auto ctx = mini_ros::init();

  auto sensor = std::make_shared<SensorNode>(ctx);
  auto monitor = std::make_shared<MonitorNode>(ctx);
  auto alarm = std::make_shared<AlarmNode>(ctx);

  mini_ros::SingleThreadedExecutor exec(ctx);
  exec.add_node(sensor);
  exec.add_node(monitor);
  exec.add_node(alarm);

  std::cout << "--- spin 2s (bước 100ms) ---\n";
  exec.spin_for(2s, 100ms);
  std::cout << "--- xong ---\n\n";

  CHECK(sensor->published() == 10);
  CHECK(monitor->received() == 10);
  CHECK(monitor->alerts() == 4);
  CHECK(alarm->count() == 4);
  CHECK(monitor->max_seen() == 29.0);
  CHECK(alarm->last_value() == 29.0);

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉 — bạn đã ghép được cả hệ thống!\n"
                            : "\nCòn lỗi, hoàn thiện các TODO rồi chạy lại.\n");
  return g_fail == 0 ? 0 : 1;
}
