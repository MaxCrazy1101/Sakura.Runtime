target("SkrEditorCore")
    set_group("03.editors")
    add_rules("qt.shared")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    -- set_kind("shared")
    add_deps("SkrLightningStorage", "QtAdvancedDocking")
    add_frameworks(qt_frameworks)
    add_includedirs("include", {public=true})
    add_files("src/*.cpp") 
    -- add_files("src/**.ui")
    add_files("include/**.h")
    add_defines("SKR_EDITOR_CORE_IMPL")