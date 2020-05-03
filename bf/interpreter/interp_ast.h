#ifndef DEV_SPIRALGERBIL_BF_INTERPRETER_INTERP_AST_H_
#define DEV_SPIRALGERBIL_BF_INTERPRETER_INTERP_AST_H_

#include "bf/compiler/ast.h"

namespace dev::spiralgerbil::bf {

void InterpAst(const ast::Tree& program_ast);

}  // namespace dev::spiralgerbil::bf

#endif  // DEV_SPIRALGERBIL_BF_INTERPRETER_INTERP_AST_H_
