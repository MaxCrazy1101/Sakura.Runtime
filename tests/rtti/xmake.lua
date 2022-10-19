shared_module("rtti-test-types", "RTTI_TEST_TYPES", engine_version)
    set_group("05.tests")
    public_dependency("SkrRT", engine_version)
    add_files("types/types.cpp")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"types/**.h", "types/**.hpp"},
        rootdir = "types/",
        api = "RTTI_TEST_TYPES"
    })

target("rtti-test")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("rtti-test-types", engine_version)
    add_files("rtti/rtti.cpp")
    add_packages("gtest")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"rtti/**.h", "rtti/**.hpp"},
        rootdir = "rtti/",
        api = "RTTI_TEST"
    })

target("sptr-test")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("rtti-test-types", engine_version)
    add_files("sptr/**.cpp")
    add_packages("gtest")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"sptr/**.h", "sptr/**.hpp"},
        rootdir = "sptr/",
        api = "SPTR_TEST"
    })