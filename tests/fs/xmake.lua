target("VFSTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_packages("catch2", {public = true})
    add_files("test/main.cpp")