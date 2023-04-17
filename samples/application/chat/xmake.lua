target("chat")
    set_languages("cxx17")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_files("main.cpp", "signal_client.cpp", "imgui_impl_sdl.cpp")
    add_deps("gamenetworkingsockets")
