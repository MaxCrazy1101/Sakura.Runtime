target("common-math-test")
    set_group("05.tests")
    set_kind("binary")
    add_deps("SkrRT")
    add_packages("gtest")
    add_files("Common/CommonMath.cpp")
    set_languages("c++17")