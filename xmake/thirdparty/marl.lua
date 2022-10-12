marl_include_dir = "$(projectdir)/thirdparty/marl/include"
marl_private_include_dir = "$(projectdir)/thirdparty/marl/include/marl"
marl_source_dir = "$(projectdir)/thirdparty/marl"

target("marl")
    set_group("00.thirdparty")
    set_kind("static")
    --set_optimize("fastest")
    add_files(marl_source_dir.."/src/**.cpp")
    if not has_config("is_msvc") then 
        add_files(marl_source_dir.."/src/**.c")
        add_files(marl_source_dir.."/src/**.S")
    end
    add_includedirs(marl_include_dir, {public = true})
    add_includedirs(marl_private_include_dir, {public = false})