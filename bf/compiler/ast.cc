#include "bf/compiler/ast.h"

#include <iomanip>

namespace dev::spiralgerbil::bf {

NodeList& Node::siblings() {
  CHECK(parent_ != nullptr);
  return parent_->children();
}

const NodeList& Node::siblings() const {
  CHECK(parent_ != nullptr);
  return parent_->children();
}

void Node::Reparent(NodeContainer* new_parent) {
  CHECK(new_parent != nullptr);
  new_parent->Add(parent_->Remove(this));
}

std::unique_ptr<Node> Node::Unparent() {
  return parent_->Remove(this);
}

std::string Node::DebugString() const {
  std::stringstream buffer;
  DebugStringPart(&buffer, 0);
  return std::move(buffer).str();
}

NodeContainer::NodeContainer(NodeList children)
    : Node(0) {
  for (auto& node : children) {
    CHECK(node.parent() == nullptr);
    node.set_parent(this);
  }
  children_ = std::move(children);
}

Node* NodeContainer::Add(std::unique_ptr<Node> node) {
  CHECK(node != nullptr);
  CHECK(node->type() != NodeType::Tree);
  node->set_parent(this);
  children().push_back(std::move(node));
  return node.release();
}

Node* NodeContainer::Insert(NodeList::iterator before, std::unique_ptr<Node> node) {
  return &*children().insert(before, std::move(node));
}

namespace ast {
namespace {

constexpr int INDENT_INCREMENT = 2;

void PrintSubnodes(std::stringstream* buffer, int indent, const NodeList& subnodes) {
  for (const auto& node : subnodes) {
    node.DebugStringPart(buffer, indent + INDENT_INCREMENT);
    *buffer << ",\n";
  }
}

void IndentPrint(std::stringstream* buffer, int indent, std::string_view output) {
  *buffer << std::setw(indent) << "" << output;
}

}  // namespace

void Tree::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Tree(\n");
  PrintSubnodes(buffer, indent, children());
  IndentPrint(buffer, indent, ")");
}

void Move::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Move ");
  *buffer << distance();
}

void Add::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Add ");
  *buffer << offset() << " " << amount();
}

void Output::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Output ");
  *buffer << offset();
}

void Input::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Input ");
  *buffer << offset();
}

void Loop::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Loop[\n");
  PrintSubnodes(buffer, indent, children());
  IndentPrint(buffer, indent, "]");
}

void Set::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "Set ");
  *buffer << offset() << " " << value();
}

void AddMul::DebugStringPart(std::stringstream* buffer, int indent) const {
  IndentPrint(buffer, indent, "AddMul ");
  *buffer << offset() << " x" << multiplier();
}

NodeList::iterator Tree::Accept(NodeVisitor* visitor, NodeList::iterator iter) {
  visitor->Visit(this);
  return NodeList::iterator();
}

NodeList::iterator Move::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }
NodeList::iterator Add::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }
NodeList::iterator Output::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }
NodeList::iterator Input::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }
NodeList::iterator Loop::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }
NodeList::iterator Set::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }
NodeList::iterator AddMul::Accept(NodeVisitor* visitor, NodeList::iterator iter) { return visitor->Visit(this, iter); }

}  // namespace ast

void NodeVisitor::Visit(ast::Tree* node) {
  VisitChildren(node);
}

NodeList::iterator NodeVisitor::Visit(ast::Loop* node, NodeList::iterator iter) {
  VisitChildren(node);
  return iter;
}

void NodeVisitor::VisitChildren(NodeContainer* node) {
  auto& children = node->children();
  for (auto iter = children.begin(); iter != children.end(); ++iter) {
    iter = iter->Accept(this, iter);
  }
}

}  // namespace dev::spiralgerbil::bf::ast
