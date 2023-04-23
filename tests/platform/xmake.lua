target("ThreadsTest")
    set_kind("binary")
    set_group("05.tests/platform")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("threads/threads.cpp")

target("MarlTest")
    set_kind("binary")
    set_group("05.tests/platform")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("marl-test/**.cpp")