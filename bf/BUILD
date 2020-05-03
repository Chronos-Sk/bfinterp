load("@rules_cc//cc:defs.bzl", "cc_binary")

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

cc_binary(
    name = "hello",
    srcs = ["hello.cc"],
    deps = [
        "@absl//absl/flags:flag",
        "@absl//absl/flags:parse",
    ],
)
