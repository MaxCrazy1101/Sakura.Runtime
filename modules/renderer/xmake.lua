target("SkrRenderer")
    set_kind("shared")
    add_deps("SkrRenderGraph", "SkrImGui")
    add_includedirs("include", {public=true})
    add_defines("SKR_RENDERER_SHARED", {public=true})
    add_defines("SKR_RENDERER_IMPL")
    add_files("src/*.cpp")