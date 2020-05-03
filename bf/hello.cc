#include <cstdio>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, hello, "", "");

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  printf("Hello %s!\n", absl::GetFlag(FLAGS_hello).c_str());
}
