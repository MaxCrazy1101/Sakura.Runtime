option("build_tools")
    set_default(true)
    set_showmenu(true)
    set_description("Toggle to build tools of SakuraRuntime")
option_end()

option("build_samples")
    set_default(true)
    set_showmenu(true)
    set_description("Toggle to build samples of SakuraRuntime")
option_end()

option("build_wasm3_samples")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build samples of wasm3")
option_end()

option("build_cgpu_samples")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build samples of CGPU")
option_end()

option("use_zig")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build samples of zig")
option_end()

option("build_rg_samples")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build samples of render graph")
option_end()

option("build_editors")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build editors of SakuraRuntime")
option_end()

option("build_tests")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build tests of SakuraRuntime")
option_end()

option("build_AAA")
    set_default(false)
    set_description("Toggle to build AAA project")
option_end()

option("use_async_codegen")
    set_default(true)
    set_description("Toggle to use async codegen")
option_end()

option("use_tracy")
    -- "auto", "enable", "disable"
    set_default("auto")
    set_values("auto", "enable", "disable")
    set_showmenu(true)
    set_description("Toggle to use tracy profile")
option_end()

option("cxx_version")
    -- "auto", "enable", "disable"
    set_default("cxx20")
    set_values("cxx17", "cxx20", "cxx23")
    set_showmenu(true)
    set_description("c++ version of project")
option_end()

option("c_version")
    -- "auto", "enable", "disable"
    set_default("c11")
    set_values("c11")
    set_showmenu(true)
    set_description("c version of project")
option_end()
