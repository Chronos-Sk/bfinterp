load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//bf:__subpackages__"])

cc_library(
    name = "interp_ast",
    srcs = ["interp_ast.cc"],
    hdrs = ["interp_ast.h"],
    deps = [
        "//bf/compiler:ast",
        "@absl//absl/base:core_headers",
    ],
)
