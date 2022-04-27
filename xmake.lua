set_project("SakuraRuntime")

add_rules("mode.debug", "mode.release")
set_languages("c11", "cxx17")

include_dir_list = {"include"}
source_list = {}
packages_list = {"vulkan"}
deps_list = {}
links_list = {}

includes("xmake/rules.lua")
includes("xmake/options_detect.lua")

includes("xmake/mimalloc.lua")
includes("xmake/boost.lua")
includes("xmake/gsl.lua")
includes("xmake/EASTL.lua")
includes("xmake/DirectXMath.lua")
includes("xmake/SDL2.lua")
includes("xmake/imgui.lua")
includes("xmake/tracy.lua")
includes("xmake/gfx-sdk.lua")
includes("xmake/wasm3.lua")
includes("xmake/parallel_hashmap.lua")
includes("xmake/FiberTaskingLib.lua")

set_warnings("all")
if (is_os("windows")) then 
    add_defines("UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    else
        set_runtimes("MDd")
    end
else 
    add_requires("vulkan")
end

target("SkrRT")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    set_kind("static")
    add_deps(deps_list)
    add_packages(packages_list, {public = true})
    add_includedirs(include_dir_list, {public = true})
    add_files(source_list)
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    add_files("src/**/*.hlsl")
    add_cxflags(project_cxflags)
    if (is_os("windows")) then 
        add_links("advapi32", "Shcore")
        add_includedirs(path.join("$(env VULKAN_SDK)", "Include"))
    end
    if (is_os("macosx")) then 
        add_mxflags(project_mxflags)
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit")
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end
    before_build(function(target)
        import("core.project.task")
        task.run("unzip-tracyclient")
        task.run("unzip-wasm3")
        task.run("unzip-gfx-sdk")
    end)
    add_links(links_list)
    add_links("TracyClient")
    add_links("m3", "uv_a", "uvwasi_a")

includes("samples/xmake.lua")
includes("tests/xmake.lua")