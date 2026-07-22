// ============================================================================
// mini_ros — Một "ROS 2 tí hon" viết bằng C++17 THUẦN (không cần cài ROS).
// Mục đích: tái hiện LUỒNG CHÍNH của rclcpp để bạn hiểu bản chất, đồng thời
// vận dụng TẤT CẢ các topic C++ modern đã học.
//
// Mỗi thành phần dưới đây được chú thích topic tương ứng:
//   [01] smart pointers   [03] lambda/std::function   [04] move/forward
//   [05] templates        [06] chrono/time            [07] STL containers
//   [08] optional/variant [09] mutex (thread-safe)    [10] class/RAII/virtual
//   [11] enum class/constexpr
//
// Khác biệt với ROS thật (đã đơn giản hoá — xem README):
//   * Thời gian là "ảo" do Executor điều khiển -> hoàn toàn tất định (deterministic).
//   * Middleware chỉ là một bus trong cùng process (không DDS, không mạng).
//   * Chỉ có SingleThreadedExecutor với vòng lặp thời gian mô phỏng.
// ============================================================================
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mini_ros {

// ============================ [06] Time =====================================
// Dùng nanoseconds làm đơn vị nội bộ; chấp nhận mọi std::chrono::duration.
using Duration = std::chrono::nanoseconds;

// ============================ [11] Logging ==================================
enum class LogLevel { Debug, Info, Warn, Error };

constexpr const char* to_cstr(LogLevel l) {   // [11] constexpr + enum class
  switch (l) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
  }
  return "?";
}

// ============================ QoS ===========================================
struct QoS {
  std::size_t depth = 10;                       // độ sâu hàng đợi (mô phỏng)
  QoS() = default;
  explicit QoS(std::size_t d) : depth(d) {}
};

// ============================ [08] Parameter ================================
// Giống rclcpp::ParameterValue: một biến giữ MỘT trong nhiều kiểu.
using ParamValue = std::variant<bool, std::int64_t, double, std::string>;

// ============================ Context (middleware bus) ======================
// Giữ toàn bộ trạng thái "graph": subscriptions, timers, hàng đợi callback.
class Context {
public:
  using ErasedMsg = std::shared_ptr<const void>;                 // [01] type erasure
  using ErasedHandler = std::function<void(const ErasedMsg&)>;   // [03]

  struct TimerEntry {
    Duration period{0};
    Duration next_fire{0};
    std::function<void()> cb;
  };

  // [07] map tên topic -> danh sách handler; hàng đợi callback chờ Executor xử lý.
  std::unordered_map<std::string, std::vector<ErasedHandler>> subs;
  std::vector<TimerEntry> timers;
  std::queue<std::function<void()>> pending;
  std::mutex mtx;                                 // [09] bảo vệ hàng đợi
  Duration now{0};                                // thời gian ảo hiện tại

  void enqueue(std::function<void()> fn) {
    std::lock_guard<std::mutex> lk(mtx);          // [09] RAII lock
    pending.push(std::move(fn));                  // [04] move
  }
};

inline std::shared_ptr<Context> init() {          // ~ rclcpp::init()
  return std::make_shared<Context>();             // [01] make_shared
}

// ============================ [05][01][04] Publisher<T> =====================
template <typename MsgT>
class Publisher {
public:
  using SharedPtr = std::shared_ptr<Publisher<MsgT>>;

  Publisher(std::shared_ptr<Context> ctx, std::string topic, QoS qos)
      : ctx_(std::move(ctx)), topic_(std::move(topic)), qos_(qos) {}

  // publish NHẬN message theo giá trị rồi MOVE vào shared_ptr -> tránh copy thừa.
  void publish(MsgT msg) {
    auto sp = std::make_shared<const MsgT>(std::move(msg));   // [04] move
    Context::ErasedMsg erased = sp;                          // upcast tới const void
    auto it = ctx_->subs.find(topic_);
    if (it == ctx_->subs.end()) return;
    for (const auto& h : it->second) {
      // Không gọi ngay: đẩy vào hàng đợi để Executor gọi (giống ROS thật).
      ctx_->enqueue([h, erased] { h(erased); });             // [03] lambda capture
    }
  }

  const std::string& topic() const { return topic_; }
  std::size_t qos_depth() const { return qos_.depth; }

private:
  std::shared_ptr<Context> ctx_;
  std::string topic_;
  QoS qos_;
};

// ============================ Subscription<T> ===============================
template <typename MsgT>
class Subscription {
public:
  using SharedPtr = std::shared_ptr<Subscription<MsgT>>;
  using Callback = std::function<void(std::shared_ptr<const MsgT>)>;   // [03]

  Subscription(std::string topic, Callback cb)
      : topic_(std::move(topic)), cb_(std::move(cb)) {}

  const std::string& topic() const { return topic_; }
  const Callback& callback() const { return cb_; }

private:
  std::string topic_;
  Callback cb_;
};

// ============================ [10][01] Node =================================
// Lớp cơ sở — bạn sẽ KẾ THỪA nó (giống class MyNode : public rclcpp::Node).
class Node : public std::enable_shared_from_this<Node> {   // [01] shared_from_this
public:
  Node(std::string name, std::shared_ptr<Context> ctx)
      : name_(std::move(name)), ctx_(std::move(ctx)) {}
  virtual ~Node() = default;                                // [10] virtual dtor

  const std::string& get_name() const { return name_; }

  // [05] create_publisher<MsgT>(...) — template theo kiểu message.
  template <typename MsgT>
  typename Publisher<MsgT>::SharedPtr
  create_publisher(const std::string& topic, const QoS& qos) {
    return std::make_shared<Publisher<MsgT>>(ctx_, topic, qos);   // [01]
  }

  // [05][03][04] create_subscription — nhận callback bất kỳ (lambda/bind/functor).
  template <typename MsgT, typename CallbackT>
  typename Subscription<MsgT>::SharedPtr
  create_subscription(const std::string& topic, const QoS& /*qos*/, CallbackT&& cb) {
    typename Subscription<MsgT>::Callback fn = std::forward<CallbackT>(cb);  // [04]
    auto sub = std::make_shared<Subscription<MsgT>>(topic, fn);
    // Đăng ký handler đã "xoá kiểu": cast ngược const void -> const MsgT.
    ctx_->subs[topic].push_back(
        [fn](const Context::ErasedMsg& m) {
          auto typed = std::static_pointer_cast<const MsgT>(m);   // [01]
          fn(typed);
        });
    return sub;
  }

  // [06] timer theo chu kỳ chrono (nhận mọi duration: 200ms, 1s, ...).
  template <typename Rep, typename Period, typename CallbackT>
  void create_wall_timer(std::chrono::duration<Rep, Period> period, CallbackT&& cb) {
    Context::TimerEntry t;
    t.period = std::chrono::duration_cast<Duration>(period);     // [06] duration_cast
    t.next_fire = ctx_->now + t.period;
    t.cb = std::forward<CallbackT>(cb);                          // [04]
    ctx_->timers.push_back(std::move(t));
  }

  // ---- [08] Parameters ----
  void declare_parameter(const std::string& name, ParamValue def) {
    if (params_.find(name) == params_.end()) params_[name] = std::move(def);
  }
  void set_parameter(const std::string& name, ParamValue v) {
    params_[name] = std::move(v);
  }
  // Trả optional: không có tham số / sai kiểu -> nullopt.
  template <typename T>
  std::optional<T> get_parameter(const std::string& name) const {
    auto it = params_.find(name);
    if (it == params_.end()) return std::nullopt;
    if (const T* p = std::get_if<T>(&it->second)) return *p;     // [08]
    return std::nullopt;
  }

  // ---- Logging ----
  void log(LogLevel lvl, const std::string& msg) const {
    std::cout << "[" << to_cstr(lvl) << "] [" << name_ << "] " << msg << "\n";
  }

protected:
  const std::shared_ptr<Context>& context() const { return ctx_; }

private:
  std::string name_;
  std::shared_ptr<Context> ctx_;
  std::map<std::string, ParamValue> params_;      // [07]
};

// ============================ [10][06][07] Executor ========================
// SingleThreadedExecutor với THỜI GIAN ẢO: mỗi bước tiến `step`, kích hoạt
// timer tới hạn, rồi rút cạn hàng đợi callback. Hoàn toàn tất định.
class SingleThreadedExecutor {
public:
  explicit SingleThreadedExecutor(std::shared_ptr<Context> ctx)
      : ctx_(std::move(ctx)) {}

  void add_node(std::shared_ptr<Node> n) { nodes_.push_back(std::move(n)); }

  template <typename D1, typename D2>
  void spin_for(D1 total_in, D2 step_in) {
    const Duration total = std::chrono::duration_cast<Duration>(total_in);
    const Duration step = std::chrono::duration_cast<Duration>(step_in);
    Duration elapsed{0};
    while (elapsed < total) {
      ctx_->now += step;
      elapsed += step;
      for (auto& t : ctx_->timers) {                 // [07] duyệt timers
        while (t.period.count() > 0 && t.next_fire <= ctx_->now) {
          t.cb();
          t.next_fire += t.period;
        }
      }
      drain();
    }
  }

  // Xử lý hết callback đang chờ (giống spin_some).
  void drain() {
    while (true) {
      std::function<void()> fn;
      {
        std::lock_guard<std::mutex> lk(ctx_->mtx);   // [09]
        if (ctx_->pending.empty()) break;
        fn = std::move(ctx_->pending.front());
        ctx_->pending.pop();
      }
      fn();
    }
  }

private:
  std::shared_ptr<Context> ctx_;
  std::vector<std::shared_ptr<Node>> nodes_;         // giữ node sống (ownership)
};

}  // namespace mini_ros
