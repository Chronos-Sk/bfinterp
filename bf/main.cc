#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "glog/logging.h"

#include "bf/compiler/ast.h"
#include "bf/compiler/optimizer.h"
#include "bf/compiler/parser.h"
#include "bf/interpreter/interp_ast.h"

ABSL_FLAG(std::string, input, "", "BF file to run.");
ABSL_FLAG(bool, print, false, "Print AST and exit.");

namespace dev::spiralgerbil::bf {
namespace {

void LoadAndRun(const std::string& filename, bool print_only) {
  std::ifstream program_file(filename);
  if (!program_file) {
    LOG(FATAL) << "Could not open file: " << filename;
  }
  std::unique_ptr<ast::Tree> program = Parse(&program_file);
  Optimize(program.get());
  if (print_only) {
    std::puts(program->DebugString().c_str());
  } else {
    InterpAst(*program);
  }
}

}  // namespace
}  // namespace dev::spiralgerbil::bf

int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(argv[0]);
  if (absl::ParseCommandLine(argc, argv).size() > 1) {
    LOG(ERROR) << "Too many arguments.";
    return -1;
  }
  dev::spiralgerbil::bf::LoadAndRun(absl::GetFlag(FLAGS_input), absl::GetFlag(FLAGS_print));
}
