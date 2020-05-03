#ifndef DEV_SPIRALGERBIL_BF_OPTIMIZER_AST_H_
#define DEV_SPIRALGERBIL_BF_OPTIMIZER_AST_H_

#include <memory>

#include "bf/compiler/ast.h"

namespace dev::spiralgerbil::bf {

void Optimize(ast::Tree* program);

void CollapseClearLoops(ast::Tree* tree);
void RemoveImpossibleLoops(ast::Tree* tree);
void CollapseAddMulLoops(ast::Tree* tree);
void ConvertToOffsets(ast::Tree* tree);

}  // dev::spiralgerbil::bf

#endif  // DEV_SPIRALGERBIL_BF_OPTIMIZER_AST_H_