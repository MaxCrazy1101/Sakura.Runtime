add_requires("lua >=5.4.4-skr")

target("BaseTest")
    set_group("05.tests")
    set_kind("binary")
    add_files("base/base.cpp")
    add_packages("gtest")
    add_packages("lua", {public = true, inherit = true})
    add_syslinks("advapi32", "Shcore", "user32", "shell32", "Ole32", {public = true})
    add_deps("SkrDependencyGraph", {public = false})
    add_links("$(buildir)/$(os)/$(arch)/$(mode)/nvapi_x64", {public = true})
    add_links("$(buildir)/$(os)/$(arch)/$(mode)/WinPixEventRuntime", {public = true})
    add_links(links_list, {public = true})
    add_packages(packages_list, {public = true})

target("CommonMathTest")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("Common/CommonMath.cpp")

target("GraphTest")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    add_packages("gtest")
    add_files("Graph/Graph.cpp")