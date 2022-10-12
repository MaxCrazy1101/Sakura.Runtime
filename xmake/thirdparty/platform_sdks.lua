task("unzip-platform-sdks")
    on_run(function ()
        import("find_sdk")
        if(os.host() == "windows") then
            find_sdk.install_lib("dstorage")
            find_sdk.install_lib("amdags")
            find_sdk.install_lib("nvapi")
            find_sdk.install_lib("nsight")
            find_sdk.install_lib("WinPixEventRuntime")
            find_sdk.install_lib("SDL2")
            find_sdk.install_lib("m3")
        end
    end)

vk_includes_dir = "$(projectdir)/SDKs/vulkan"

target("vulkan")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(vk_includes_dir, {public=true})