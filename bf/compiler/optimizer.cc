#include "bf/compiler/optimizer.h"

#include "absl/container/flat_hash_map.h"

#include "glog/logging.h"

namespace dev::spiralgerbil::bf {

void Optimize(ast::Tree* program) {
  RemoveImpossibleLoops(program);
  CollapseClearLoops(program);
  CollapseAddMulLoops(program);
}

void RemoveImpossibleLoops(ast::Tree* tree) {
  auto& children = tree->children();
  auto iter = children.begin();
  for (auto end = children.end(); iter != end && iter->type() == NodeType::Loop; ++iter);
  children.erase(children.begin(), iter);

  class : public NodeVisitor {
   public:
    using NodeVisitor::Visit;
    NodeList::iterator Visit(ast::Loop* node, NodeList::iterator iter) {
      if (node->children().empty()) {
        return node->siblings().erase(iter) - 1;
      } else if (!node->first() && (iter-1)->type() == NodeType::Loop) {
        return node->siblings().erase(iter) - 1;
      }
      VisitChildren(node);
      return iter;
    }
  } visitor;
  visitor.Visit(tree);
}

void CollapseClearLoops(ast::Tree* tree) {
  class : public NodeVisitor {
   public:
    using NodeVisitor::Visit;
    NodeList::iterator Visit(ast::Loop* node, NodeList::iterator iter) override {
      auto& children = node->children();
      if (children.size() == 1 && children[0].type() == NodeType::Add) {
        iter.replace<ast::Set>(0);
      } else {
        VisitChildren(node);
      }
      return iter;
    }
  } visitor;
  visitor.Visit(tree);
}

void CollapseAddMulLoops(ast::Tree* tree) {
  class : public NodeVisitor {
   public:
    using NodeVisitor::Visit;
    NodeList::iterator Visit(ast::Loop* node, NodeList::iterator iter) override {
      if (AttemptReplace(node, iter)) {
        VisitChildren(node);
      }
      return iter;
    }

   private:
    bool AttemptReplace(ast::Loop* node, NodeList::iterator iter) {
      auto& children = node->children();
      if (children.empty()) {
        return false;
      }
      int net_offset = 0;
      absl::flat_hash_map<int, int> multipliers;
      for (const Node& child : children) {
        switch (child.type()) {
          case NodeType::Move:
            net_offset += static_cast<const ast::Move&>(child).distance();
            break;
          case NodeType::Add:
            multipliers[net_offset] += static_cast<const ast::Add&>(child).amount();
            break;
          default:
            return true;
        }
      }
      if (net_offset != 0 || multipliers[0] != -1) {
        return false;
      }
      // Valid to replace.
      children.clear();
      multipliers.erase(0);
      for (const auto& [offset, multiplier] : multipliers) {
        children.emplace_back<ast::AddMul>(offset, multiplier);
      }
      children.emplace_back<ast::Set>(0);
      return false;
    }
  } visitor;
  visitor.Visit(tree);
}

}  // dev::spiralgerbil::bf
