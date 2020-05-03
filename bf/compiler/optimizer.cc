#include "bf/compiler/optimizer.h"

#include "absl/container/flat_hash_map.h"

#include "glog/logging.h"

namespace dev::spiralgerbil::bf {

void Optimize(ast::Tree* program) {
  RemoveImpossibleLoops(program);
  CollapseClearLoops(program);
  CollapseAddMulLoops(program);
  ConvertToOffsets(program);
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

void ConvertToOffsets(ast::Tree* tree) {
  class OffsetVisitor : public NodeVisitor {
   public:
    using NodeVisitor::Visit;

    NodeList::iterator Visit(ast::Move* node, NodeList::iterator iter) override {
      current_offset += node->distance();
      return iter;
    }

    NodeList::iterator Visit(ast::Add* node, NodeList::iterator iter) override {
      replacement.emplace_back<ast::Add>(node->amount(), current_offset);
      return iter;
    }

    NodeList::iterator Visit(ast::Output* node, NodeList::iterator iter) override {
      replacement.emplace_back<ast::Output>(current_offset);
      return iter;
    }

    NodeList::iterator Visit(ast::Input* node, NodeList::iterator iter) override {
      replacement.emplace_back<ast::Input>(current_offset);
      return iter;
    }

    NodeList::iterator Visit(ast::Loop* node, NodeList::iterator iter) override {
      FlushOffset();
      OffsetVisitor visitor;
      visitor.VisitChildren(node);
      replacement.emplace_back<ast::Loop>(visitor.Build());
      return iter;
    }

    NodeList::iterator Visit(ast::Set* node, NodeList::iterator iter) {
      replacement.emplace_back<ast::Set>(node->value(), current_offset);
      return iter;
    }

    NodeList::iterator Visit(ast::AddMul* node, NodeList::iterator iter) {
      FlushOffset();
      replacement.emplace_back<ast::AddMul>(node->offset(), node->multiplier());
      return iter;
    }

    NodeList Build() {
      FlushOffset();
      NodeList other;
      other.swap(replacement);
      return other;
    }

   private:
    NodeList replacement;
    int current_offset = 0;

    void FlushOffset() {
      if (current_offset != 0) {
        replacement.emplace_back<ast::Move>(current_offset);
        current_offset = 0;
      }
    }
  } visitor;
  visitor.Visit(tree);
  tree->children() = visitor.Build();
}

}  // dev::spiralgerbil::bf
