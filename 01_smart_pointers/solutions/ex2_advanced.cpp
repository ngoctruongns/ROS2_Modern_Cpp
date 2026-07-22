// ============================================================================
// Topic 01 — Bài 2 (ADVANCED) — LỜI GIẢI / SOLUTION
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

class Node : public std::enable_shared_from_this<Node> {
public:
  std::string name;
  std::weak_ptr<Registry> registry;   // weak: tránh vòng lặp Registry<->Node

  explicit Node(std::string n) : name(std::move(n)) {}
  void announce();
};

class Registry {
public:
  std::vector<std::shared_ptr<Node>> nodes;
  void add(const std::shared_ptr<Node>& n) { nodes.push_back(n); }
  size_t size() const { return nodes.size(); }
};

void Node::announce() {
  if (auto reg = registry.lock()) {        // registry còn sống?
    reg->add(shared_from_this());          // shared_ptr tới chính node này
  }
}

int main() {
  auto reg = std::make_shared<Registry>();

  {
    auto n1 = std::make_shared<Node>("talker");
    n1->registry = reg;                    // weak_ptr nhận từ shared_ptr
    n1->announce();

    CHECK(reg->size() == 1);
    CHECK(reg->nodes[0]->name == "talker");
    CHECK(n1.use_count() >= 2);
  }

  std::weak_ptr<Node> observer = reg->nodes.empty() ? std::weak_ptr<Node>{}
                                                    : std::weak_ptr<Node>(reg->nodes[0]);
  CHECK(!observer.expired());
  reg->nodes.clear();
  CHECK(observer.expired());

  std::cout << (g_fail == 0 ? "\nTẤT CẢ PASS 🎉\n" : "\nCòn lỗi.\n");
  return g_fail == 0 ? 0 : 1;
}
