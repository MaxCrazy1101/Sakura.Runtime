target("VFSTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("test/main.cpp")