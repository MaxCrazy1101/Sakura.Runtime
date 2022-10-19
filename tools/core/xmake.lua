shared_module("SkrTool", "TOOL", engine_version)
    set_group("02.tools")
    add_files("src/**.cpp")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public = true})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/",
        api = "TOOL"
    })
    -- add_rules("c++.unity_build", {batchsize = default_unity_batch_size})