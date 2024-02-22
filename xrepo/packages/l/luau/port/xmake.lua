set_languages("c++17")

target("Luau.Common")
    set_kind("headeronly")
    add_includedirs("luau/Common/include", {public = true})
    add_headerfiles("luau/Common/include/**.h")

target("Luau.VM")
    set_kind("static")
    set_optimize("fastest")
    add_deps("Luau.Common")
    add_defines("LUA_USE_LONGJMP=1", "LUA_API=extern\"C\"", {public = true})
    add_includedirs("luau/VM/include", {public = true})
    add_headerfiles("luau/VM/include/**.h")
    add_files("luau/VM/src/**.cpp")
    if is_plat("windows") then
        add_files("luau/VM/src/lvmexecute.cc", {force = {cxflags = "/d2ssa-pre-"}})
    else
        add_files("luau/VM/src/lvmexecute.cc")
    end
    add_cxflags("-fno-math-errno", { public = true, tools = {"clang_cl", "clang"} })

target("Luau.Ast")
    set_kind("static")
    set_optimize("fastest")
    add_deps("Luau.Common")
    add_includedirs("luau/Ast/include", {public = true})
    add_headerfiles("luau/Ast/include/**.h")
    add_files("luau/Ast/src/**.cpp")

target("Luau.Compiler")
    set_kind("static")
    set_optimize("fastest")
    add_deps("Luau.Ast")
    add_defines("LUACODE_API=extern\"C\"", {public = true})
    add_includedirs("luau/Compiler/include", {public = true})
    add_headerfiles("luau/Compiler/include/**.h")
    add_files("luau/Compiler/src/**.cpp")