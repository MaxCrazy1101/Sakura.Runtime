shared_module("SkrRenderGraph", "SKR_RENDER_GRAPH", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    set_pcxxheader("src/pch.hpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_includedirs("include", {public=true})
    add_files("src/graphviz/*.cpp", {unity_ignored = true})
    add_files("src/frontend/*.cpp")
    add_files("src/backend/*.cpp", {unity_group = "backend"})
    add_files("src/phases/*.cpp", {unity_group = "backend"})