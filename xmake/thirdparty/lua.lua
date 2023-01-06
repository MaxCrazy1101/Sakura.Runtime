lua_include_dir = "$(projectdir)/thirdparty/lua/include"
lua_include_dir_private = "$(projectdir)/thirdparty/lua/include/lua"
lua_source_dir = "$(projectdir)/thirdparty/lua/src"

target("lua")
    set_group("00.thirdparty")
    set_kind("static")
    add_includedirs(lua_include_dir, {public = true})
    add_includedirs(lua_include_dir_private, {public = false})
    if (is_os("windows") and not is_mode("asan")) then 
        set_kind("headeronly")
        add_linkdirs(sdk_libs_dir, {public=true})
        if (is_mode("release")) then
            add_links("lua", {public=true} )
        else
            add_links("luad", {public=true} )
        end
    else
        set_kind("static")
        add_files(lua_source_dir.."/*.c")
    end