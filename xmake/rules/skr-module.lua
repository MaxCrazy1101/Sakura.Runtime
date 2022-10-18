option("module_as_objects")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

if(has_config("module_as_objects")) then
    add_defines("MODULE_AS_OBJECTS")
end

rule("skr.shared")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.shared", "api")
        target:set("kind", "shared")
        target:add("defines", api.."_SHARED", {public=true})
        target:add("defines", api.."_IMPL")
    end)

rule("skr.module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.module", "api")
        if(has_config("module_as_objects")) then
            target:set("kind", "object")
        else
            target:set("kind", "shared")
            target:add("defines", api.."_SHARED", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)

rule("skr.static_module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.static_module", "api")
        if(has_config("module_as_objects")) then
            target:set("kind", "object")
        else
            target:set("kind", "static")
            target:add("defines", api.."_STATIC", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)