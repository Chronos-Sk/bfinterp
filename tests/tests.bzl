""" BF test macros. """

def bf_integration_test(input, size = "small"):
    native.sh_test(
        name = "bf_integration_test__" + input,
        srcs = ["bf_integration_test.sh"],
        args = [input + ".bf"],
        data = [
            input + ".bf",
            "//bf",
        ],
        size = size,
    )
