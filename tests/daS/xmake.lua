add_requires("daScript 2023.4.25-skr.10")

-- simple interpret
target("daSTest0")
    set_kind("binary")
    set_group("05.tests/daS")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTest0/**.cpp")

-- AOT
target("daSTest1")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("@daScript/AOT", {
        outdir = "./scripts"
    })
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTest1/**.das")
    add_files("daSTest1/**.cpp")

-- AOT
target("daSTest2")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("@daScript/Hybrid", {
        outdir = "./scripts"
    })
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTest1/**.das", {aot = true})
    add_files("daSTest1/**.cpp")