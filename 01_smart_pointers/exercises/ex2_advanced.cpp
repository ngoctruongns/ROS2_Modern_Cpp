// ============================================================================
// Topic 01 — Smart Pointers — Bài 2 (ADVANCED)
// Mục tiêu: weak_ptr để phá vòng lặp tham chiếu + enable_shared_from_this.
// Bối cảnh: mô phỏng một "Node" ROS 2 tự đăng ký chính nó vào một Registry,
//           và Node giữ tham chiếu ngược tới Registry (phải là weak để tránh leak).
//
//   ./build/ex2_advanced   (đề)   |   ./build/sol_ex2_advanced (lời giải)
// ============================================================================
#include <iostream>
#include <memory>
#include <string>
#include <vector>

static int g_fail = 0;
#define CHECK(cond)                                                            \
  do {                                                                         \
    if (cond) std::cout << "[PASS] " << #cond << "\n";                         \
    else { std::cout << "[FAIL] " << #cond << " (line " << __LINE__ << ")\n"; ++g_fail; } \
  } while (0)

class Registry;

// Node phải kế thừa enable_shared_from_this để tự trả shared_ptr tới chính nó.
class Node : public std::enable_shared_from_this<Node> {
public:
  std::string name;
  // TODO 1: khai báo tham chiếu ngược tới Registry sao cho KHÔNG tạo vòng lặp
  //         giữ nhau sống mãi. (Gợi ý: weak_ptr, không phải shared_ptr.)
  // std::weak_ptr<Registry> registry;

  explicit Node(std::string n) : name(std::move(n)) {}

  // Đăng ký chính node này vào registry đang trỏ tới.
  void announce();  // định nghĩa sau khi Registry đã đầy đủ
};

class Registry {
public:
  std::vector<std::shared_ptr<Node>> nodes;  // Registry SỞ HỮU các node

  void add(const std::shared_ptr<Node>& n) { nodes.push_back(n); }
  size_t size() const { return nodes.size(); }
};

void Node::announce() {
  // TODO 2: nếu registry còn sống, thêm shared_ptr-tới-chính-mình vào registry.
  //         Dùng shared_from_this() và .lock() cho weak_ptr.
  // if (auto reg = registry.lock()) reg->add(shared_from_this());
}

int main() {
  auto reg = std::make_shared<Registry>();

  {
    auto n1 = std::make_shared<Node>("talker");
    // TODO 3: gán n1->registry = reg (weak sẽ tự chuyển từ shared).
    // n1->registry = reg;
    n1->announce();

    CHECK(reg->size() == 1);
    CHECK(!reg->nodes.empty() && reg->nodes[0]->name == "talker");
    // n1 được registry giữ -> use_count >= 2 (n1 + trong vector)
    CHECK(n1.use_count() >= 2);
  }

  // Kiểm tra không leak vòng lặp: node vẫn sống vì registry giữ nó,
  // nhưng nếu ta clear registry thì node phải được huỷ (count về 0).
  std::weak_ptr<Node> observer = reg->nodes.empty() ? std::weak_ptr<Node>{}
                                                    : std::weak_ptr<Node>(reg->nodes[0]);
  CHECK(!observer.expired());   // còn sống khi registry còn giữ
  reg->nodes.clear();
  CHECK(observer.expired());    // registry buông -> node bị huỷ (không leak)

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
