target("threads-test")
    set_kind("binary")
    set_group("05.tests/platform")
    add_deps("SkrRT")
    add_packages("gtest")
    add_files("threads/threads.cpp")
    set_languages("c++17")