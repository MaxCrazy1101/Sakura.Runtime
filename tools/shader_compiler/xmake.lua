shared_module("SkrShaderCompiler", "SKR_SHADER_COMPILER", engine_version)
    set_group("02.tools")
    add_rules("utils.install-libs", { libnames = {"dxc"} })
    add_includedirs("include", {public=true})
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrTool", engine_version)
    add_files("src/**.cpp")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrShaderCompiler",
        api = "SKR_SHADER_COMPILER"
    })