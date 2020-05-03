load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//bf:__subpackages__"])

cc_library(
    name = "poly_list",
    hdrs = ["poly_list.h"],
    deps = [
        "@boost//:iterator",
    ],
)

cc_library(
    name = "ast",
    srcs = ["ast.cc"],
    hdrs = ["ast.h"],
    deps = [
        ":poly_list",
        "@glog",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":ast",
    ],
)

cc_library(
    name = "optimizer",
    srcs = ["optimizer.cc"],
    hdrs = ["optimizer.h"],
    deps = [
        ":ast",
        "@absl//absl/container:flat_hash_map",
        "@glog",
    ],
)
