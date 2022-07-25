set_project("Sakura.Runtime")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_moduledirs("xmake/modules")

includes("xmake/options.lua")

set_languages("c11", "cxx17")

include_dir_list = {"include"}
source_list = {}
packages_list = {}
deps_list = {}
defs_list = {}
links_list = {}
generator_list = {}

includes("xmake/options_detect.lua")
includes("xmake/rules.lua")
includes("xmake/thirdparty.lua")
includes("tools/codegen/xmake.lua")

set_warnings("all")

if (is_os("windows")) then 
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    else
        set_runtimes("MDd")
    end
end

target("SkrRT") 
    add_deps(deps_list)
    add_defines(defs_list, {public = true})
    add_rules("skr.module", {api = "RUNTIME"})
    add_packages(packages_list, {public = true})
    add_includedirs(include_dir_list, {public = true})
    add_files(source_list)
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    add_cxflags(project_cxflags, {public = true, force = true})
    -- runtime compile definitions
    after_load(function (target,  opt)
        if (target:get("kind") == "shared") then
            target:add("defines", "MI_SHARED_LIB", "EA_DLL", {public = true})
            target:add("defines", "MI_SHARED_LIB_EXPORT", "EASTL_API=EA_EXPORT", "EASTL_EASTDC_API=EA_EXPORT")
        end
    end)
    -- link system libs/frameworks
    if (is_os("windows")) then 
        add_links("advapi32", "Shcore", "user32", "shell32", "Ole32", {public = true})
    end
    if (is_os("macosx")) then 
        add_mxflags(project_cxflags, project_mxflags, {public = true, force = true})
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit", {public = true})
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end
    -- unzip & link sdks
    before_build(function(target)
        import("core.project.task")
        task.run("unzip-tracyclient")
        --task.run("unzip-wasm3")
        task.run("unzip-gfx-sdk")
    end)
    add_links(links_list, {public = true})
    -- boost ctx
    do
        local platform = "x86_64"
        local os = "ms"
        local abi = "pe"
        local asm = "masm.asm"

        if is_arch("x86_64", "amd64", "x64") then 
            if is_config("pointer-size", "8") then 
                platform = "x86_64"
            else 
            platform = "i386"
            end
        end
        if is_arch("i386") then 
            platform = "i386"
        end
        if is_arch("arm.*") then 
            if is_config("pointer-size", "4") then 
                platform = "arm64"
            else
                platform = "arm"
            end
            os = "aapcs"
        end
        if is_arch("aarch64.*") then 
            platform = "arm64"
            os = "aapcs"
        end
        if is_os("windows") then
            if os == nil then
                os = "ms"
            end
            abi = "pe"
            if has_config("is_msvc") then 
                asm = "masm.asm"
            else 
                asm = "gas.S"
            end
        elseif is_os("linux") then 
            if os == nil then
                os = "sysv"
            end 
            abi = "elf"
            asm = "gas.S"
        elseif is_os("macosx") then
            os = "sysv"
            abi = "macho"
            asm = "gas.S"
        else 
            error("Unsupported os")
        end
        local canary = ""
        if is_config("ftl_fiber_canary_bytes") then 
            canary = "canary_"
        else 
            canary = ""
        end
        local file = canary..platform.."_"..os.."_"..abi.."_"..asm
        if asm == "masm.asm" and platform == "i386" then
            add_asflags("/safeseh")
        end
        add_files("$(projectdir)/thirdparty/boost_context/asm/make_"..file)
        add_files("$(projectdir)/thirdparty/boost_context/asm/jump_"..file)
    end
    if has_config("is_unix") then 
        add_syslinks("pthread")
    end 
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/*.cpp")
    add_cxflags(project_cxflags, {public = true})
target_end()

includes("modules/xmake.lua")
if(has_config("build_samples")) then
    includes("samples/xmake.lua")
end
if(has_config("build_tools")) then
    includes("tools/xmake.lua")
end
if(has_config("build_tests")) then
    includes("tests/xmake.lua")
end