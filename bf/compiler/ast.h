#ifndef DEV_SPIRALGERBIL_BF_COMPILER_AST_H_
#define DEV_SPIRALGERBIL_BF_COMPILER_AST_H_

#include <algorithm>
#include <deque>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "glog/logging.h"

#include "bf/compiler/poly_list.h"

namespace dev::spiralgerbil::bf {

enum class NodeType {
  // Original
  Tree,
  Move,
  Add,
  Output,
  Input,
  Loop,
  // Extensions
  Set,
  AddMul,
};

class Node;
class NodeContainer;
class NodeVisitor;

using NodeList = PolyList<Node>;

class Node {
 public:
  // Parent takes ownership, if not null.
  Node(NodeContainer* parent = nullptr);

  // Not copyable or movable, to avoid slicing.
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
  Node(Node&&) = delete;
  Node& operator=(Node&&) = delete;

  virtual ~Node() {}

  NodeContainer* parent() { return parent_; }
  const NodeContainer* parent() const { return parent_; }
  NodeList& siblings();
  const NodeList& siblings() const;

  bool first() const { return this == &siblings().front(); }
  bool last() const { return this == &siblings().back(); }

  virtual NodeType type() const = 0;

  void Reparent(NodeContainer* new_parent);
  [[nodiscard]] std::unique_ptr<Node> Unparent();

  std::string DebugString() const;
  virtual void DebugStringPart(std::stringstream* buffer, int indent) const = 0;
  
  virtual NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) = 0;

  virtual bool operator==(const Node& other) const = 0;
  bool operator!=(const Node& other) const { return !(*this == other); } 

 private:
  NodeContainer* parent_;

  void set_parent(NodeContainer* new_parent) { parent_ = new_parent; }

  friend NodeContainer;
};

class NodeContainer : public Node {
 public:
  using Node::Node;
  NodeContainer(NodeList children, NodeContainer* parent = nullptr);

  NodeList& children() { return children_; }
  const NodeList& children() const { return children_; }

  Node* Add(std::unique_ptr<Node> node);
  Node* Insert(NodeList::iterator before, std::unique_ptr<Node> node);

  bool operator==(const Node& other) const override { return this == &other; }

  template <typename V>
  [[nodiscard]] std::unique_ptr<V> Remove(V* node) {
    CHECK(node->parent() == this);
    node->set_parent(nullptr);
    return children().remove(node);
  }

 private:
  NodeList children_;
};

namespace ast {

class Tree final : public NodeContainer {
 public:
  Tree() : NodeContainer(nullptr) {}
  explicit Tree(NodeList children) : NodeContainer(std::move(children)) {}

  NodeType type() const { return NodeType::Tree; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
};

class Move final : public Node {
 public:
  explicit Move(int distance, NodeContainer* parent = nullptr)
      : Node(parent), distance_(distance) {}

  int distance() const { return distance_; }

  NodeType type() const { return NodeType::Move; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
  bool operator==(const Node& other) const override;

 private:
  const int distance_;
};

class Add final : public Node {
 public:
  explicit Add(int amount, NodeContainer* parent = nullptr)
      : Node(parent), amount_(amount) {}

  int amount() const { return amount_; }

  NodeType type() const { return NodeType::Add; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
  bool operator==(const Node& other) const override;

 private:
  const int amount_;
};

class Output final : public Node {
 public:
  explicit Output(NodeContainer* parent = nullptr) : Node(parent) {}
  
  NodeType type() const { return NodeType::Output; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
  bool operator==(const Node& other) const override { return type() == other.type(); }
};

class Input final : public Node {
 public:
  explicit Input(NodeContainer* parent = nullptr) : Node(parent) {}

  NodeType type() const { return NodeType::Input; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
  bool operator==(const Node& other) const override { return type() == other.type(); }
};

class Loop final : public NodeContainer {
 public:
  explicit Loop(NodeContainer* parent = nullptr)
      : NodeContainer(parent) {}
  explicit Loop(NodeList children, NodeContainer* parent = nullptr)
      : NodeContainer(std::move(children), parent) {}

  NodeType type() const { return NodeType::Loop; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
};

class Set final : public Node {
 public:
  explicit Set(int value, NodeContainer* parent = nullptr)
      : Node(parent), value_(value) {}

  int value() const { return value_; }

  NodeType type() const { return NodeType::Set; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
  bool operator==(const Node& other) const override;

 private:
  const int value_;
};

class AddMul final : public Node {
 public:
  explicit AddMul(int offset, int multiplier, NodeContainer* parent = nullptr)
      : Node(parent), offset_(offset), multiplier_(multiplier) {}

  int offset() const { return offset_; }
  int multiplier() const { return multiplier_; }

  NodeType type() const { return NodeType::AddMul; }
  void DebugStringPart(std::stringstream* buffer, int indent) const override;
  NodeList::iterator Accept(NodeVisitor* visitor, NodeList::iterator iter) override;
  bool operator==(const Node& other) const override;

 private:
  const int offset_;
  const int multiplier_;
};

}  // namespace ast

class NodeVisitor {
 public:
  void Visit(ast::Tree* node);
  virtual NodeList::iterator Visit(ast::Move* node, NodeList::iterator iter) { return iter; }
  virtual NodeList::iterator Visit(ast::Add* node, NodeList::iterator iter) { return iter; }
  virtual NodeList::iterator Visit(ast::Output* node, NodeList::iterator iter) { return iter; }
  virtual NodeList::iterator Visit(ast::Input* node, NodeList::iterator iter) { return iter; }
  virtual NodeList::iterator Visit(ast::Loop* node, NodeList::iterator iter);
  virtual NodeList::iterator Visit(ast::Set* node, NodeList::iterator iter) { return iter; }
  virtual NodeList::iterator Visit(ast::AddMul* node, NodeList::iterator iter) { return iter; }

 protected:
  void VisitChildren(NodeContainer* node);
};

}  // namespace dev::spiralgerbil::bf

#endif  // DEV_SPIRALGERBIL_BF_COMPILER_AST_H_
