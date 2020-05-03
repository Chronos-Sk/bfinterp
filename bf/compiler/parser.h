#ifndef DEV_SPIRALGERBIL_BF_COMPILER_PARSER_H_
#define DEV_SPIRALGERBIL_BF_COMPILER_PARSER_H_

#include <istream>
#include <memory>

#include "bf/compiler/ast.h"

namespace dev::spiralgerbil::bf {

std::unique_ptr<ast::Tree> Parse(std::istream* input_stream);

}  // namespace dev::spiralgerbil::bf

#endif  // DEV_SPIRALGERBIL_BF_COMPILER_PARSER_H_
