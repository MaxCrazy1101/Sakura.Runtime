if (os.host() == "windows") then
    wasm3_includes_dir = "$(projectdir)/thirdparty/wasm3"
    shared_module("SkrWASM", "SKR_WASM", engine_version)
        set_group("01.modules")
        public_dependency("SkrRT", engine_version)
        add_includedirs("include", {public=true})
        add_files("src/build.*.c", "src/build.*.cpp")
        add_rules("utils.install-libs", { libnames = { "m3" } })
        add_links("m3", "uv_a", "uvwasi_a", {public = true})
        add_includedirs(wasm3_includes_dir, {public = true})
end