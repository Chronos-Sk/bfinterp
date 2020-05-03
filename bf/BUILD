load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//:__subpackages__"])

cc_binary(
    name = "bf",
    srcs = ["main.cc"],
    deps = [
        "//bf/compiler:ast",
        "//bf/compiler:optimizer",
        "//bf/compiler:parser",
        "//bf/interpreter:interp_ast",
        "@absl//absl/flags:flag",
        "@absl//absl/flags:parse",
        "@absl//absl/strings:str_format",
        "@absl//absl/types:span",
        "@glog",
    ],
)
