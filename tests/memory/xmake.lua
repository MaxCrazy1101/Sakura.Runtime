target("SSMTest")
    set_kind("binary")
    set_group("05.tests/memory")
    public_dependency("SkrCore", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("SSM/*.cpp")