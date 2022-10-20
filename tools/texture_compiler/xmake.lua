target("ISPCTextureCompressor")
    set_group("02.tools")
    set_kind("static")
    set_policy("build.across_targets_in_parallel", false)
    add_rules("utils.ispc")
    add_files("src/**.ispc")

shared_module("SkrTextureCompiler", "TEXTURE_COMPILER", engine_version)
    set_group("02.tools")
    add_includedirs("include", {public=true})
    public_dependency("SkrTool", engine_version)
    add_deps("ISPCTextureCompressor")
    add_files("src/**.cpp")
