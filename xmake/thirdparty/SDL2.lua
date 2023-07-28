sdl2_includes_dir = "$(projectdir)/thirdparty/SDL2"

target("SkrRoot")
    add_includedirs(sdl2_includes_dir, {public = true})

if (is_os("windows")) then 
    table.insert(links_list, "SDL2")
elseif (is_os("macosx") or is_os("linux")) then
    add_requires("libsdl", {configs = {shared = true}})
    table.insert(packages_list, "libsdl")
else

end