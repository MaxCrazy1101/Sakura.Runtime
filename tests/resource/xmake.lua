target("ResourceDesign")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_files("design/main.cpp")
    set_languages("c++17")