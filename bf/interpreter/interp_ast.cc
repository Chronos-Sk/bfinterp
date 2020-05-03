#include "bf/interpreter/interp_ast.h"

#include <cassert>
#include <cstdio>
#include <vector>

#include "absl/base/optimization.h"

namespace dev::spiralgerbil::bf {
namespace {

using MemType = uint16_t;
constexpr int MemSize = 30000;

struct Context {
  std::vector<MemType> memory;
  std::vector<MemType>::iterator mem_ptr;

  Context() : memory(MemSize, 0), mem_ptr(memory.begin()) {}
};

void InterpAst_rec(const NodeContainer& container, Context* context) {
  for (const auto& node : container.children()) {
    const auto mem_target = context->mem_ptr + node.offset();
    switch (node.type()) {
      case NodeType::Move:
        context->mem_ptr += static_cast<const ast::Move&>(node).distance();
        break;
      case NodeType::Add:
        *mem_target += static_cast<const ast::Add&>(node).amount();
        break;
      case NodeType::Output:
        std::putchar(*mem_target);
        break;
      case NodeType::Input:
        *mem_target = std::getchar();
        break;
      case NodeType::Loop:
        while (*context->mem_ptr) {
          InterpAst_rec(static_cast<const ast::Loop&>(node), context);
        }
        break;
      case NodeType::Set:
        *mem_target = static_cast<const ast::Set&>(node).value();
        break;
      case NodeType::AddMul: {
        const ast::AddMul& addmul = static_cast<const ast::AddMul&>(node);
        *mem_target += *context->mem_ptr * addmul.multiplier();
        break;
      }
      default:
        ABSL_INTERNAL_ASSUME(false);
    }
  }
}

}  // namespace

void InterpAst(const ast::Tree& program_ast) {
  Context context;
  InterpAst_rec(program_ast, &context);
}

}  // namespace dev::spiralgerbil::bf

